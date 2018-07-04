#include "Ogre_glTF_skeletonImporter.hpp"
#include "Ogre_glTF_common.hpp"
#include "Ogre_glTF_internal_utils.hpp"
#include <OgreOldSkeletonManager.h>
#include <OgreSkeleton.h>
#include <OgreOldBone.h>
#include <OgreLogManager.h>

int Ogre_glTF_skeletonImporter::skeletonID = 0;

void Ogre_glTF_skeletonImporter::addChidren(const std::string& skinName, const std::vector<int>& childs, Ogre::v1::OldBone* parent)
{
	OgreLog("Bone " + std::to_string(parent->getHandle()) + " has " + std::to_string(childs.size()) + " children");
	for(auto child : childs)
	{
		const auto& node = model.nodes[child];
		OgreLog("Node name is " + node.name + "!");
		auto toFloat = [](double n) { return static_cast<float>(n); };

		std::array<float, 3> translation {};
		std::array<float, 4> rotation {};

		internal_utils::container_double_to_float(node.translation, translation);
		internal_utils::container_double_to_float(node.rotation, rotation);

		auto bone = skeleton->getBone(child - offset);
		if(!bone)
		{
			throw std::runtime_error("could not get bone " + std::to_string(child - offset));
		}

		parent->addChild(bone);

		bone->setPosition(Ogre::Vector3 { translation.data() });
		bone->setOrientation(Ogre::Quaternion { rotation.data() });

		//auto bone = parent->createChild(child - offset, Ogre::Vector3{ translation.data() }, Ogre::Quaternion{ rotation.data() });
		Ogre::LogManager::getSingleton().logMessage("Bone pointer value : " + std::to_string(std::size_t(bone)));

		addChidren(skinName + std::to_string(child), model.nodes[child].children, bone);
	}
}

void Ogre_glTF_skeletonImporter::loadBoneHierarchy(const tinygltf::Skin& skin, Ogre::v1::OldBone* rootBone, const std::string& name)
{
	const auto& node	 = model.nodes[skin.joints[0]];
	const auto& skeleton = model.nodes[skin.skeleton];

	std::array<float, 3> translation {};
	std::array<float, 4> rotation {};

	internal_utils::container_double_to_float(skeleton.translation, translation);
	internal_utils::container_double_to_float(skeleton.rotation, rotation);

	rootBone->setPosition(Ogre::Vector3 { translation.data() });
	rootBone->setOrientation(Ogre::Quaternion { rotation.data() });

	addChidren(name, node.children, rootBone);
}

Ogre_glTF_skeletonImporter::Ogre_glTF_skeletonImporter(tinygltf::Model& input) :
 model { input }
{
}

Ogre::v1::SkeletonPtr Ogre_glTF_skeletonImporter::getSkeleton()
{
	const auto& skins = model.skins;

	assert(skins.size() > 0);
	const auto skin = skins[0];

	std::string skeletonName;

	if(!skin.name.empty())
		skeletonName = skin.name;
	else
		skeletonName = "unnamedSkeleton" + std::to_string(skeletonID++);

	OgreLog("First skin name is " + skeletonName);

	skeleton = Ogre::v1::OldSkeletonManager::getSingleton().getByName(skeletonName);
	if(skeleton)
	{
		OgreLog("Found in the skeleton manager");
		return skeleton;
	}

	skeleton = Ogre::v1::OldSkeletonManager::getSingleton().create(skeletonName,
																   Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
																   true);

	if(!skeleton)
	{
		throw std::runtime_error("Coudn't create skeletion for skin" + skeletonName);
	}

	offset = skin.joints[0];

	OgreLog("skin.skeleton = " + std::to_string(skin.skeleton));
	OgreLog("first joint : " + std::to_string(offset));
	for(auto joint : skin.joints)
	{
		const auto name = model.nodes[joint].name;
		OgreLog("joint " + std::to_string(joint) + " name: " + name);
		skeleton->createBone((!name.empty()) ? name : skeletonName + std::to_string(joint - offset), joint - offset);
	}

	auto rootBone = skeleton->getBone(0);
	loadBoneHierarchy(skin, rootBone, skeletonName);

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

	struct keyFrame
	{
		double timePoint {};
		double weights {};
		Ogre::Quaternion rotation {};
		Ogre::Vector3 position, scale {};
	};

	using keyFrameList			 = std::vector<keyFrame>;
	using tinygltfJointNodeIndex = int;

	std::unordered_map<tinygltfJointNodeIndex, keyFrameList> boneIndexedKeyFrames;
	auto getAnimationLenght = [](const keyFrameList& l) {
		if(l.empty()) return 0.0;
		//const auto comp = [](const keyFrame& lhs, const keyFrame& rhs)
		//{
		//	return lhs.timePoint < rhs.timePoint;
		//};

		////has to be sorted
		//if(!std::is_sorted(l.begin(), l.end(), comp))
		//	std::sort(l.begin(), l.end(), comp);

		return l.back().timePoint;
	};

	using channelList = std::vector<std::reference_wrapper<tinygltf::AnimationChannel>>;
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
				const auto joint					   = channel.target_node;
				const auto& boneRawAnnimationChannelIt = boneRawAnimationChannels.find(channel.target_node);
				if(boneRawAnnimationChannelIt == boneRawAnimationChannels.end())
				{
					boneRawAnimationChannels[joint];
				}

				boneRawAnimationChannels[joint].push_back((channel));
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

				{
					auto translationIt = std::find_if(channels.begin(), channels.end(), [](const tinygltf::AnimationChannel& c) {
						return c.target_path == "translation";
					});

					auto rotationIt = std::find_if(channels.begin(), channels.end(), [](const tinygltf::AnimationChannel& c) {
						return c.target_path == "rotation";
					});

					auto scaleIt = std::find_if(channels.begin(), channels.end(), [](const tinygltf::AnimationChannel& c) {
						return c.target_path == "scale";
					});

					auto weightsIt = std::find_if(channels.begin(), channels.end(), [](const tinygltf::AnimationChannel& c) {
						return c.target_path == "weights";
					});

					if(translationIt != channels.end())
					{
						translation = &(*translationIt).get();
					}

					if(rotationIt != channels.end())
					{
						rotation = &(*rotationIt).get();
					}

					if(scaleIt != channels.end())
					{
						scale = &(*scaleIt).get();
					}

					if(weightsIt != channels.end())
					{
						weights = &(*weightsIt).get();
					}
				}

				maxLen = getAnimationLenght(keyFrames);
			}
		}
	}

	return skeleton;
}
