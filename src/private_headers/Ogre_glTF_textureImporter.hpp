#pragma once

#include "tiny_gltf.h"
#include <unordered_map>
#include <OgreTextureGpu.h>
#include <OgreTextureGpuManager.h>
#include <OgrePixelFormatGpuUtils.h>
#include <OgreTextureBox.h>
#include <OgrePixelFormatGpu.h>
#include "OgreBitwise.h"
#include <OgreHlms.h>
#include <OgreHlmsPbs.h>


namespace Ogre_glTF
{

	///Import textures described in glTF into Ogre
	class textureImporter
	{
		///List of the loaded basic textures
		std::unordered_map<int, Ogre::TextureGpu *> mLoadedTextures;

		///Static counter to make unique texture name. Incremented by constructor
		static size_t mId;

		///Reference to the tinygltf
		tinygltf::Model& mModel;

		Ogre::TextureGpuManager* mTextureManager;

		std::vector<std::uint8_t> mPixelBuffer;


	public:
		///Construct the texture importer object. Inrement the id counter
		/// \param input reference to the model that we are loading
		textureImporter(tinygltf::Model& input);

		Ogre::TextureGpu* getTexture(
			int glTFTextureIndex, 
			Ogre::PbsTextureTypes texType, 
			Ogre::PixelFormatGpu inputPixelFormat=Ogre::PixelFormatGpu::PFG_RGBA8_UNORM);

		void preparePixelBuffer(Ogre::uint32 componentOffset, const tinygltf::Image* sourceImage);
	};
}