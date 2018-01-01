#pragma once

#include "tiny_gltf.h"
#include <unordered_map>
#include <OgreTexture.h>

class Ogre_glTF_textureImporter
{
	std::unordered_map<int, Ogre::TexturePtr> loadedTextures;
	static size_t id;
	tinygltf::Model& model;
	void loadTexture(const tinygltf::Texture& texture);

public:
	Ogre_glTF_textureImporter(tinygltf::Model& input);
	void loadTextures();
	Ogre::TexturePtr getTexture(int glTFTextureSourceID);
};