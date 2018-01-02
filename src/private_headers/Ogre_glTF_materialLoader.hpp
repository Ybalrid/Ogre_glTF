#pragma once
#include "tiny_gltf.h"
#include <OgreHlms.h>
#include <OgreHlmsPbs.h>
class Ogre_glTF_textureImporter;

///Load material information from a model inside Ogre, provide you a datablock to set to an Ogre::Item object
class Ogre_glTF_materialLoader
{
	///Reference to the texture importer that deal with the current model's material
	Ogre_glTF_textureImporter& textureImporter;
	///The model
	tinygltf::Model& model;

	///Get the texture index from a material field
	/// \param content Any material field from a tinygltf::Model
	template<typename T> int getTextureIndex(const T& content) const
	{
		const auto it = content.second.json_double_value.find("index");
		if (it != std::end(content.second.json_double_value))
		{
			return int(it->second);
		}
		return -1;
	}

	///Return true if the texture index is valid
	bool isTextureIndexValid(int textureIndex) const;

	///Set the diffuse texture (baseColorTexture)
	/// \param block datablock to set
	/// \param value gltf texture index
	void setBaseColorTexture(Ogre::HlmsPbsDatablock* block, int value) const;
	///Set the metalness and roughness textures (metalRoughTexture)
	/// \param block datablock to set
	/// \param value gltf texture index
	void setMetalRoughTexture(Ogre::HlmsPbsDatablock* block, int value) const;
	///Set the normal texture
	/// \param block datablock to set
	/// \param value gltf texture index
	void setNormalTexture(Ogre::HlmsPbsDatablock* block, int value) const;
	///Set the occlusion texure (AFAIK, Ogre don't use them, so this does nothing)
	/// \param block datablock to set
	/// \param value gltf texture index
	void setOcclusionTexture(Ogre::HlmsPbsDatablock* block, int value) const;

public:
	///Construct the material loader
	/// \param input model to load material from
	/// \param textureInterface the texture importer to get Ogre texture from
	Ogre_glTF_materialLoader(tinygltf::Model& input, Ogre_glTF_textureImporter& textureInterface);
	///Get the material (the HlmsDatablock)
	Ogre::HlmsDatablock* getDatablock() const;
};