#include "Ogre_glTF_skeletonImporter.hpp"
#include "Ogre_glTF_common.hpp"
#include "Ogre_glTF_internal_utils.hpp"
#include <OgreOldSkeletonManager.h>
#include <OgreSkeleton.h>
#include <OgreOldBone.h>
#include <OgreLogManager.h>
#include <OgreKeyFrame.h>

using namespace Ogre_glTF;

int skeletonImporter::skeletonID = 0;

void skeletonImporter::addChidren(const std::string& skinName, const std::vector<int>& childs, Ogre::v1::OldBone* parent, const std::vector<int>& joints)
{
	//OgreLog("Bone " + std::to_string(parent->getHandle()) + " has " + std::to_string(childs.size()) + " children");

	for(auto child : childs)
	{
		const auto& node = model.nodes[child];
		//OgreLog("Node name is " + node.name + "!");

		auto bone = skeleton->getBone(nodeToJointMap[child]);
		if(!bone)
		{
			throw std::runtime_error("could not get bone " + std::to_string(bone->getHandle()));
		}

		parent->addChild(bone);

		auto bindMatrix = inverseBindMatrices[nodeToJointMap[child]].inverseAffine();

		Ogre::Vector3 translation, scale;
		Ogre::Quaternion rotaiton;

		bindMatrix.decomposition(translation, scale, rotaiton);

		bone->setPosition(parent->convertWorldToLocalPosition(translation));
		bone->setOrientation(parent->convertWorldToLocalOrientation(rotaiton));
		bone->setScale(parent->_getDerivedScale() / scale);

		addChidren(skinName + std::to_string(child), model.nodes[child].children, bone, joints);
	}
}

void skeletonImporter::loadBoneHierarchy(const tinygltf::Skin& skin, Ogre::v1::OldBone* rootBone, const std::string& name)
{
	const auto& node	 = model.nodes[skin.joints[0]];
	const auto& skeleton = model.nodes[skin.skeleton];

	std::array<float, 3> translation{ 0 }, scale{ 0 };
	std::array<float, 4> rotation{ 0 };
	internal_utils::container_double_to_float(skeleton.translation, translation);
	if(skeleton.scale.size() == 3)
		internal_utils::container_double_to_float(skeleton.scale, scale);
	internal_utils::container_double_to_float(skeleton.rotation, rotation);

	Ogre::Vector3 trans  = Ogre::Vector3{ translation.data() };
	Ogre::Quaternion rot = Ogre::Quaternion{ rotation[3], rotation[0], rotation[1], rotation[2] };
	Ogre::Vector3 sc	 = Ogre::Vector3{ scale.data() };

	if(sc.isZeroLength()) sc = Ogre::Vector3::UNIT_SCALE;

	rootBone->setPosition(trans);
	rootBone->setOrientation(rot);
	rootBone->setScale(sc);

	std::stringstream rootBoneXformLog;
	rootBoneXformLog << "rootBone " << trans << " " << rot;
	OgreLog(rootBoneXformLog);

	addChidren(name, node.children, rootBone, skin.joints);
}

skeletonImporter::skeletonImporter(tinygltf::Model& input) :
 model{ input }
{
}

void skeletonImporter::loadTimepointFromSamplerToKeyFrame(int bone, int frameID, int& count, keyFrame& animationFrame, tinygltf::AnimationSampler& sampler)
{
	auto& input				 = model.accessors[sampler.input];
	count					 = input.count;
	auto& bufferView		 = model.bufferViews[input.bufferView];
	auto& buffer			 = model.buffers[bufferView.buffer];
	unsigned char* dataStart = buffer.data.data() + bufferView.byteOffset + input.byteOffset;
	const size_t byteStride  = input.ByteStride(bufferView);

	assert(input.type == TINYGLTF_TYPE_SCALAR); //Need to be a scalar, since it's a timepoint
	float data;
	if(input.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
	{
		data = *reinterpret_cast<float*>(dataStart + frameID * byteStride);
	}
	else if(input.componentType == TINYGLTF_COMPONENT_TYPE_DOUBLE)
	{
		data = static_cast<float>(*reinterpret_cast<double*>(dataStart + frameID * byteStride));
	}

	if(animationFrame.timePoint < 0)
		animationFrame.timePoint = data;
	else if(animationFrame.timePoint != data)
	{
		throw std::runtime_error("Missmatch of timecode while loading an animation keyframe for bone joint " + std::to_string(bone) + "\n"
																																	  "read from file : "
								 + std::to_string(data) + " while animationFrame recorded " + std::to_string(animationFrame.timePoint));
	}
}

void skeletonImporter::loadVector3FromSampler(int frameID, int& count, tinygltf::AnimationSampler& sampler, Ogre::Vector3& vector)
{
	auto& output			 = model.accessors[sampler.output];
	count					 = output.count;
	auto& bufferView		 = model.bufferViews[output.bufferView];
	auto& buffer			 = model.buffers[bufferView.buffer];
	unsigned char* dataStart = buffer.data.data() + bufferView.byteOffset + output.byteOffset;
	const size_t byteStride  = output.ByteStride(bufferView);

	assert(output.type == TINYGLTF_TYPE_VEC3); //Need to be a 3D vectorsince it's a translation vector

	if(output.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
	{
		vector = Ogre::Vector3(reinterpret_cast<float*>(dataStart + frameID * byteStride));
	}
	else if(output.componentType == TINYGLTF_COMPONENT_TYPE_DOUBLE) //need double to float conversion
	{
		std::array<float, 3> vectFloat{};
		std::array<double, 3> vectDouble{};

		memcpy(vectDouble.data(), reinterpret_cast<double*>(dataStart + frameID * byteStride), 3 * sizeof(double));
		internal_utils::container_double_to_float(vectDouble, vectFloat);

		vector = Ogre::Vector3(vectFloat.data());
	}
}

void skeletonImporter::loadQuatFromSampler(int frameID, int& count, tinygltf::AnimationSampler& sampler, Ogre::Quaternion& quat) const
{
	auto& output			 = model.accessors[sampler.output];
	count					 = output.count;
	auto& bufferView		 = model.bufferViews[output.bufferView];
	auto& buffer			 = model.buffers[bufferView.buffer];
	unsigned char* dataStart = buffer.data.data() + bufferView.byteOffset + output.byteOffset;
	const size_t byteStride  = output.ByteStride(bufferView);

	assert(output.type == TINYGLTF_TYPE_VEC4); //Need to be a 4D vectorsince it's a translation vector

	if(output.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
	{
		float* quat_data = reinterpret_cast<float*>(dataStart + frameID * byteStride);
		quat			 = Ogre::Quaternion(
			quat_data[3],
			quat_data[0],
			quat_data[1],
			quat_data[2]);
	}
	else if(output.componentType == TINYGLTF_COMPONENT_TYPE_DOUBLE) //need double to float conversion
	{
		std::array<float, 4> vectFloat{};
		std::array<double, 4> vectDouble{};

		memcpy(vectDouble.data(), reinterpret_cast<double*>(dataStart + frameID * byteStride), 3 * sizeof(double));
		internal_utils::container_double_to_float(vectDouble, vectFloat);

		quat = Ogre::Quaternion(vectFloat[3], vectFloat[0], vectFloat[1], vectFloat[2]);
	}
}

void skeletonImporter::detectAnimationChannel(const channelList& channels, tinygltf::AnimationChannel*& translation, tinygltf::AnimationChannel*& rotation, tinygltf::AnimationChannel*& scale, tinygltf::AnimationChannel*& weights) const
{
	const auto translationIt = std::find_if(channels.begin(), channels.end(), [](const tinygltf::AnimationChannel& c) {
		return c.target_path == "translation";
	});
	if(translationIt != channels.end())
		translation = &(*translationIt).get();

	const auto rotationIt = std::find_if(channels.begin(), channels.end(), [](const tinygltf::AnimationChannel& c) {
		return c.target_path == "rotation";
	});
	if(rotationIt != channels.end())
		rotation = &(*rotationIt).get();

	const auto scaleIt = std::find_if(channels.begin(), channels.end(), [](const tinygltf::AnimationChannel& c) {
		return c.target_path == "scale";
	});
	if(scaleIt != channels.end())
		scale = &(*scaleIt).get();

	const auto weightsIt = std::find_if(channels.begin(), channels.end(), [](const tinygltf::AnimationChannel& c) {
		return c.target_path == "weights";
	});
	if(weightsIt != channels.end())
		weights = &(*weightsIt).get();
}

void skeletonImporter::loadKeyFrameDataFromSampler(const tinygltf::Animation& animation,
															 int bone,
															 tinygltf::AnimationChannel* translation,
															 tinygltf::AnimationChannel* rotation,
															 tinygltf::AnimationChannel* scale,
															 tinygltf::AnimationChannel* weights,
															 int frameID,
															 int& count,
															 keyFrame& animationFrame)
{
	if(translation)
	{
		auto sampler = animation.samplers[translation->sampler];
		loadTimepointFromSamplerToKeyFrame(bone, frameID, count, animationFrame, sampler);
		loadVector3FromSampler(frameID, count, sampler, animationFrame.position);
	}
	if(rotation)
	{
		auto sampler = animation.samplers[rotation->sampler];
		loadTimepointFromSamplerToKeyFrame(bone, frameID, count, animationFrame, sampler);
		loadQuatFromSampler(frameID, count, sampler, animationFrame.rotation);
	}
	if(scale)
	{
		auto sampler = animation.samplers[scale->sampler];
		loadTimepointFromSamplerToKeyFrame(bone, frameID, count, animationFrame, sampler);
		loadVector3FromSampler(frameID, count, sampler, animationFrame.scale);
	}
	if(weights)
	{
		auto sampler = animation.samplers[weights->sampler];
		loadTimepointFromSamplerToKeyFrame(bone, frameID, count, animationFrame, sampler);
		//TODO load the scalar... but well, we don't do anything with that in a skeletal animation, so... do nothing
	}
}

void skeletonImporter::loadKeyFrames(const tinygltf::Animation& animation, int bone, keyFrameList& keyFrames, tinygltf::AnimationChannel* translation, tinygltf::AnimationChannel* rotation, tinygltf::AnimationChannel* scale, tinygltf::AnimationChannel* weights)
{
	bool endOfTimeLine = false;
	int frameID		   = 0;
	int count		   = 0;
	while(!endOfTimeLine)
	{
		keyFrame animationFrame;

		loadKeyFrameDataFromSampler(animation, bone, translation, rotation, scale, weights, frameID, count, animationFrame);

		keyFrames.push_back(animationFrame);
		++frameID;
		if(frameID >= count)
			endOfTimeLine = true;
	}
}

void skeletonImporter::loadSkeletonAnimations(const tinygltf::Skin skin, const std::string& skeletonName)
{
	//List all the animations that own at least one channel that target one of the bones of our skeleton
	OgreLog("Searching for animations for skeleton " + skeleton->getName());
	std::vector<std::reference_wrapper<tinygltf::Animation>> animations;
	for(auto& animation : model.animations)
	{
		for(const auto& channel : animation.channels)
		{
			if(std::find(skin.joints.begin(), skin.joints.end(), channel.target_node) != skin.joints.end())
			{
				//animation is targeting our skeleton, just save that information
				animations.emplace_back(animation);
				break;
			}
		}
	}

	std::unordered_map<tinygltfJointNodeIndex, keyFrameList> boneIndexedKeyFrames;

	const auto getAnimationLenght = [](const keyFrameList& l) {
		if(l.empty()) return 0.0f;
		return l.back().timePoint;
	};

	std::unordered_map<tinygltfJointNodeIndex, channelList> boneRawAnimationChannels;

	if(!animations.empty())
	{
		int i = 0;
		for(auto animation_rw : animations)
		{
			//Get animation
			auto animation			  = animation_rw.get();
			std::string animationName = animation.name;
			if(animation.name.empty())
				animationName = skeletonName + "Animation" + std::to_string(i++);

			OgreLog("parsing channels for animation " + animationName);

			float maxLen = 0;
			for(auto& channel : animation.channels)
			{
				const auto joint					   = nodeToJointMap[channel.target_node];
				const auto& boneRawAnnimationChannelIt = boneRawAnimationChannels.find(joint);
				if(boneRawAnnimationChannelIt == boneRawAnimationChannels.end())
					boneRawAnimationChannels[joint];

				boneRawAnimationChannels[joint].push_back(channel);
			}

			//from here, bones -> channel map has been built;
			for(auto& boneChannels : boneRawAnimationChannels)
			{
				auto bone	 = boneChannels.first;
				auto channels = boneChannels.second;

				keyFrameList keyFrames;

				tinygltf::AnimationChannel* translation = nullptr;
				tinygltf::AnimationChannel* rotation	= nullptr;
				tinygltf::AnimationChannel* scale		= nullptr;
				tinygltf::AnimationChannel* weights		= nullptr;

				detectAnimationChannel(channels, translation, rotation, scale, weights);
				loadKeyFrames(animation, bone, keyFrames, translation, rotation, scale, weights);

				//here, we have a list of all key frames for one bone
				boneIndexedKeyFrames[bone] = keyFrames;
				maxLen					   = getAnimationLenght(keyFrames);
			}

			//Create animation
			auto ogreAnimation = skeleton->createAnimation(animationName, maxLen);
			ogreAnimation->setInterpolationMode(Ogre::v1::Animation::InterpolationMode::IM_LINEAR);

			//For each bone's list of keyframes
			for(auto& keyFrameForBone : boneIndexedKeyFrames)
			{
				//Get the bone index
				const auto boneIndex	 = keyFrameForBone.first;
				const auto ogreBoneIndex = boneIndex;

				//Add a node to the animation track
				auto nodeAnimTrack = ogreAnimation->createOldNodeTrack(ogreBoneIndex);
				auto bone		   = skeleton->getBone(boneIndex);

				//for each keyframe
				for(auto& keyFrame : keyFrameForBone.second)
				{
					//Add a transform to apply
					Ogre::v1::TransformKeyFrame* transformKeyFrame = nodeAnimTrack->createNodeKeyFrame(keyFrame.timePoint);

					//Set the data
					transformKeyFrame->setRotation(bone->getOrientation().Inverse() * keyFrame.rotation);
					transformKeyFrame->setTranslate(bone->getPosition() - keyFrame.position);
					transformKeyFrame->setScale(keyFrame.scale / bone->getScale());
				}
			}

			//Need to use the keyframe 0 as base keyframe
		}
	}
}

void recurse(const tinygltf::Model& m, int node, std::vector<int>& output)
{
	output.push_back(node);
	for(auto child : m.nodes[node].children)
	{
		recurse(m, child, output);
	}
}

std::vector<int> traversal(const tinygltf::Model& m, int node)
{
	std::vector<int> o;
	recurse(m, node, o);
	return o;
}

Ogre::v1::SkeletonPtr skeletonImporter::getSkeleton(const std::string& name)
{
	const auto& skins = model.skins;
	assert(!skins.empty());

	//TODO, take the skin of a specific mesh...
	const auto& firstSkin = skins.front();

	const std::string skeletonName = name + (!firstSkin.name.empty() ? firstSkin.name : "unnamedSkeleton" + std::to_string(skeletonID++));
	OgreLog("First skin name is " + skeletonName);

	//Get skeleton
	skeleton = Ogre::v1::OldSkeletonManager::getSingleton().getByName(skeletonName);
	if(skeleton)
	{
		//OgreLog("Found in the skeleton manager");
		return skeleton;
	}

	//Create new skeleton
	skeleton = Ogre::v1::OldSkeletonManager::getSingleton().create(skeletonName,
																   Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
																   true);

	if(!skeleton) throw std::runtime_error("Coudn't create skeletion for skin" + skeletonName);

	//OgreLog("skin.skeleton = " + std::to_string(firstSkin.skeleton));
	//OgreLog("first joint : " + std::to_string(firstSkin.joints.front()));
	{
		const auto inverseBindMatricesID		= firstSkin.inverseBindMatrices;
		const auto& inverseBindMatricesAccessor = model.accessors[inverseBindMatricesID];
		const auto& bufferView					= model.bufferViews[inverseBindMatricesAccessor.bufferView];
		const auto byteStride					= inverseBindMatricesAccessor.ByteStride(bufferView);
		const auto& buffer						= model.buffers[bufferView.buffer];
		const unsigned char* dataStart			= buffer.data.data() + bufferView.byteOffset + inverseBindMatricesAccessor.byteOffset;

		assert(inverseBindMatricesAccessor.count == firstSkin.joints.size());
		assert(inverseBindMatricesAccessor.type == TINYGLTF_TYPE_MAT4);

		for(int i = 0; i < inverseBindMatricesAccessor.count; ++i)
		{
			if(inverseBindMatricesAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
			{
				const float* floatMatrix = reinterpret_cast<const float*>(dataStart + i * byteStride);
				inverseBindMatrices.push_back(Ogre::Matrix4(
												  floatMatrix[0],
												  floatMatrix[1],
												  floatMatrix[2],
												  floatMatrix[3],
												  floatMatrix[4],
												  floatMatrix[5],
												  floatMatrix[6],
												  floatMatrix[7],
												  floatMatrix[8],
												  floatMatrix[9],
												  floatMatrix[10],
												  floatMatrix[11],
												  floatMatrix[12],
												  floatMatrix[13],
												  floatMatrix[14],
												  floatMatrix[15])
												  .transpose());
			}
			else if(inverseBindMatricesAccessor.componentType == TINYGLTF_COMPONENT_TYPE_DOUBLE)
			{
				//Double to float conversion
				std::array<double, 4 * 4> doubleMatrix{};
				std::array<float, 4 * 4> floatMatrix{};
				memcpy(doubleMatrix.data(), reinterpret_cast<const double*>(dataStart + i * byteStride), 4 * 4 * sizeof(double));
				internal_utils::container_double_to_float(doubleMatrix, floatMatrix);
				inverseBindMatrices.push_back(Ogre::Matrix4(
												  floatMatrix[0],
												  floatMatrix[1],
												  floatMatrix[2],
												  floatMatrix[3],
												  floatMatrix[4],
												  floatMatrix[5],
												  floatMatrix[6],
												  floatMatrix[7],
												  floatMatrix[8],
												  floatMatrix[9],
												  floatMatrix[10],
												  floatMatrix[11],
												  floatMatrix[12],
												  floatMatrix[13],
												  floatMatrix[14],
												  floatMatrix[15])
												  .transpose());
			}

			assert(inverseBindMatrices.back().isAffine());
		}
	}

	//Build the "node to joint map". In the vertex buffer, proprery "JOINT_0" refer to the joints that affect a particular vertex of the skined mesh.
	//To refer to theses joints, it refer to the index of the node in the skin.joints array.
	//We need to be able to get the index for each of theses joints in the array easilly, so we are builind a dictionarry to be able to reverse-search them
	for(int i = 0; i < firstSkin.joints.size(); ++i)
	{

		//Get the index in the "node" array in the glTF's JSON
		const auto jointNode = firstSkin.joints[i];

		//Record in the dictionary the joint node-> index
		nodeToJointMap[jointNode] = i;

		//Get the name (if possible)
		const auto name = model.nodes[jointNode].name;

		//Create bone with index "i"
		auto bone = skeleton->createBone(!name.empty() ? name : skeletonName + std::to_string(i), i);
	}

	loadBoneHierarchy(firstSkin, skeleton->getBone(nodeToJointMap[firstSkin.skeleton]), skeletonName);
	skeleton->setBindingPose();
	loadSkeletonAnimations(firstSkin, skeletonName);

	return skeleton;
}
