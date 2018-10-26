#pragma once
#include "tiny_gltf.h"
#include <OgreHlms.h>
#include <OgreHlmsPbs.h>

namespace Ogre_glTF
{

	///Foward declare the textureImporter
	class textureImporter;

	///Load material information from a model inside Ogre, provide you a datablock to set to an Ogre::Item object
	class materialLoader
	{
		///Reference to the texture importer that deal with the current model's material
		textureImporter& textureImporterRef;
		///The model
		tinygltf::Model& model;

		static Ogre::Vector3 convertColor(const tinygltf::ColorValue& color);

		///Set the diffuse color of the material
		/// \param block datablock to set
		/// \param color the color diffused by the material
		void setBaseColor(Ogre::HlmsPbsDatablock* block, Ogre::Vector3 color) const;

		///Set the metallness of the material
		/// \param block datablock to set
		/// \param value floating point value that represent metalness of the surface
		void setMetallicValue(Ogre::HlmsPbsDatablock* block, Ogre::Real value) const;
		///Set the roughness of the material
		/// \param block datablock to set
		/// \param value floating point value that represent metalness of the surface
		void setRoughnesValue(Ogre::HlmsPbsDatablock* block, Ogre::Real value) const;
		///Set the emissive of the material
		/// \param block datablock to set
		/// \param color floating point value that represent metalness of the surface
		void setEmissiveColor(Ogre::HlmsPbsDatablock* block, Ogre::Vector3 color) const;

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

		///Set the emissive texture
		/// \param block datablock to set
		/// \param value gltf texture index
		void setEmissiveTexture(Ogre::HlmsPbsDatablock* block, int value) const;

	public:
		///Construct the material loader
		/// \param input model to load material from
		/// \param textureInterface the texture importer to get Ogre texture from
		materialLoader(tinygltf::Model& input, textureImporter& textureInterface);
		///Get the material (the HlmsDatablock)
		Ogre::HlmsDatablock* getDatablock( size_t index ) const;
	};
}