#pragma once

#include <tiny_gltf.h>
#include <OgrePrerequisites.h>
#include <OgreOldBone.h>

class Ogre_glTF_skeletonImporter
{
	///Const reference to the model
	tinygltf::Model& model;

	///Recurisve fucntion : Create a bone for each children, and each children's children...
	/// \param childs array contaning the indices of the childrens
	/// \param parent a pointer to a bone that is part of the skeleton we are creating
	void addChidren(const std::string& skinName, const std::vector<int>& childs, Ogre::v1::OldBone* parent);

	///Get the "skeleton node" from the skin object, call add children on it
	/// \param skin tinigltf skin object we are loading
	/// \param rootBone a freshly created bone object from an Ogre::SkeletonPtr
	void loadBoneHierarchy(const tinygltf::Skin& skin, Ogre::v1::OldBone* rootBone);

public:
	///Construct the skeleton importer
	/// \param input model where the skeleton data is loaded from
	Ogre_glTF_skeletonImporter(tinygltf::Model& input);
	///Return the constructed skeleton pointer
	Ogre::v1::SkeletonPtr getSkeleton();
};
