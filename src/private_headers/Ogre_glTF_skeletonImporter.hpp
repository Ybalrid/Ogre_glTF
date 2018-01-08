#pragma once

#include <tiny_gltf.h>
#include <OgrePrerequisites.h>

class Ogre_glTF_skeletonImporter
{
	tinygltf::Model& model;

public:
	Ogre_glTF_skeletonImporter(tinygltf::Model& input);
	Ogre::v1::SkeletonPtr getSkeleton();
};
