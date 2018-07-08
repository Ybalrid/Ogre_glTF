#pragma once

#include <tiny_gltf.h>
#include <OgrePrerequisites.h>
#include <OgreOldBone.h>

class Ogre_glTF_skeletonImporter
{


	///Reference to the model
	tinygltf::Model& model;


	using tinygltfJointNodeIndex = int;

	///number to increment when creating strings for skeleton with no names in glTF files
	static int skeletonID;

	///Pointer to the skeleton object we are currently working on.
	Ogre::v1::SkeletonPtr skeleton;


	///Recurisve fucntion : Create a bone for each children, and each children's children...
	/// \param skinName name of the skin
	/// \param childs array contaning the indices of the childrens
	/// \param parent a pointer to a bone that is part of the skeleton we are creating
	void addChidren(const std::string& skinName, const std::vector<int>& childs, Ogre::v1::OldBone* parent, const std::vector<int>& joints);

	///Get the "skeleton node" from the skin object, call add children on it
	/// \param skin tinigltf skin object we are loading
	/// \param rootBone a freshly created bone object from an Ogre::SkeletonPtr
	/// \param name Name of the skeleton we are loading
	void loadBoneHierarchy(const tinygltf::Skin& skin, Ogre::v1::OldBone* rootBone, const std::string& name);

	///Represent a keyframe as laded from tinygltf, but converted to Ogre objects
	struct keyFrame
	{
		float timePoint { -1 };
		float weights {};
		Ogre::Quaternion rotation {};
		Ogre::Vector3 position, scale {};
	};

	///Vector of keyframes
	using keyFrameList = std::vector<keyFrame>;
	///Tinygltf
	///Type for a list of animation channels
	using channelList = std::vector<std::reference_wrapper<tinygltf::AnimationChannel>>;
	using nodeIndexConversionMap = std::unordered_map<tinygltfJointNodeIndex, tinygltfJointNodeIndex>;

	nodeIndexConversionMap nodeToJointMap;

	std::vector<Ogre::Matrix4> inverseBindMatrices;

	///Load from an animation channel sampler the time data, and ore it inside the keyframe
	void loadTimepointFromSamplerToKeyFrame(int bone, int frameID, int& count, keyFrame& animationFrame, tinygltf::AnimationSampler& sampler);

	///Load vector of 3 doubles or 3 floats from the animation sampler, and store them into a vector
	void loadVector3FromSampler(int frameID, int& count, tinygltf::AnimationSampler& sampler, Ogre::Vector3& vector);

	///Load vector of 4 doubles or 4 floats from the animation sampler, and store them into a quaternion
	void loadQuatFromSampler(int frameID, int& count, tinygltf::AnimationSampler& sampler, Ogre::Quaternion& quat);

	///Goes throught the list of animation channel, and assign the 4 given pointers to the one that correspond to it
	void detectAnimationChannel(const channelList& channels, tinygltf::AnimationChannel*& translation, tinygltf::AnimationChannel*& rotation, tinygltf::AnimationChannel*& scale, tinygltf::AnimationChannel*& weights);

	///Load keyframe data from the samplers of each AnimationChannel into a KeyFrame object
	void loadKeyFrameDataFromSampler(const tinygltf::Animation& animation, int bone, tinygltf::AnimationChannel* translation, tinygltf::AnimationChannel* rotation, tinygltf::AnimationChannel* scale, tinygltf::AnimationChannel* weights, int frameID, int& count, keyFrame& animationFrame);

	///Load all keyframes for the animation
	void loadKeyFrames(const tinygltf::Animation& animation, int bone, keyFrameList& keyFrames, tinygltf::AnimationChannel* translation, tinygltf::AnimationChannel* rotation, tinygltf::AnimationChannel* scale, tinygltf::AnimationChannel* weights);

	///All all animation for the skeleton
	void loadSkeletonAnimations(tinygltf::Skin skin, std::string skeletonName);



public:
	///Construct the skeleton importer
	/// \param input model where the skeleton data is loaded from
	Ogre_glTF_skeletonImporter(tinygltf::Model& input);

	///Return the constructed skeleton pointer
	Ogre::v1::SkeletonPtr getSkeleton();
};
