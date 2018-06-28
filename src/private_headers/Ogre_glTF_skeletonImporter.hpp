#pragma once

#include <tiny_gltf.h>
#include <OgrePrerequisites.h>
#include <OgreOldBone.h>

class Ogre_glTF_skeletonImporter
{
	///Reference to the model
	tinygltf::Model& model;

	///Bone indexes starts from 0, but they are part of the whole scene hierarchy in glTF. We need to make them indexed from zero. 
	///This is simply done by substracting the node number of the first joint to all of the "joint" from glTF when giving them to Ogre
	///In the vertex data, the bone index where also indexed from 0, so everything matches up. 
	int offset = 0;

	///number to increment when creating strings for skeleton with no names in glTF files
	static int skeletonID;

	///Pointer to the skeleton object we are currently working on.
	Ogre::v1::SkeletonPtr skeleton;

	///Recurisve fucntion : Create a bone for each children, and each children's children...
	/// \param skinName name of the skin
	/// \param childs array contaning the indices of the childrens
	/// \param parent a pointer to a bone that is part of the skeleton we are creating
	void addChidren(const std::string& skinName, const std::vector<int>& childs, Ogre::v1::OldBone* parent);

	///Get the "skeleton node" from the skin object, call add children on it
	/// \param skin tinigltf skin object we are loading
	/// \param rootBone a freshly created bone object from an Ogre::SkeletonPtr
	/// \param name Name of the skeleton we are loading
	void loadBoneHierarchy(const tinygltf::Skin& skin, Ogre::v1::OldBone* rootBone, const std::string& name);

public:
	///Construct the skeleton importer
	/// \param input model where the skeleton data is loaded from
	Ogre_glTF_skeletonImporter(tinygltf::Model& input);
	///Return the constructed skeleton pointer
	Ogre::v1::SkeletonPtr getSkeleton();
};
