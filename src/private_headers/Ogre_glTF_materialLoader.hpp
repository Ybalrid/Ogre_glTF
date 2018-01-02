#pragma once
#include "tiny_gltf.h"
#include <OgreHlms.h>
#include <OgreHlmsPbs.h>
class Ogre_glTF_textureImporter;

class Ogre_glTF_materialLoader
{
	Ogre_glTF_textureImporter& textureImporter;
	tinygltf::Model& model;

	void setBaseColorTexture(Ogre::HlmsPbsDatablock* block, int value) const;
	void setMetalRoughTexture(Ogre::HlmsPbsDatablock* block, int value) const;
	void setNormalTexture(Ogre::HlmsPbsDatablock* block, int value) const;
	void setOcclusionTexture(Ogre::HlmsPbsDatablock* block, int value) const;

public:
	Ogre_glTF_materialLoader(tinygltf::Model& input, Ogre_glTF_textureImporter& textureInterface);
	Ogre::HlmsDatablock* getDatablock() const;
};
