#pragma once

#include <tiny_gltf.h>
#include <OgrePrerequisites.h>
#include <OgreOldBone.h>

class Ogre_glTF_skeletonImporter
{
	tinygltf::Model& model;

	///Recurisve fucntion : Create a bone for each children, and each children's children...
	/// \param childs array contaning the indices of the childrens
	/// \param parent a pointer to a bone that is part of the skeleton we are creating
	void addChidren(const std::string& skinName, const std::vector<int>& childs, Ogre::v1::OldBone* parent);

	///Get the "skeleton node" from the skin object, call add children on it
	void loadBoneRierarchy(const tinygltf::Skin& skin, Ogre::v1::OldBone* rootBone);
public:
	Ogre_glTF_skeletonImporter(tinygltf::Model& input);
	Ogre::v1::SkeletonPtr getSkeleton();
};