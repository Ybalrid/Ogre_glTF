#include "Ogre_glTF_textureImporter.hpp"
#include <OgreLogManager.h>
#include <OgreTextureManager.h>
#include <OgreTexture.h>
#include <OgreImage.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreRenderTexture.h>

//#define DEBUG_TEXTURE_OUTPUT

size_t Ogre_glTF_textureImporter::id{ 0 };

inline void OgreLog(const std::string& message)
{
	Ogre::LogManager::getSingleton().logMessage(message);
}

void Ogre_glTF_textureImporter::loadTexture(const tinygltf::Texture& texture)
{
	auto textureManager = Ogre::TextureManager::getSingletonPtr();
	const auto& image = model.images[texture.source];
	const auto name = "glTF_texture_" + image.name + std::to_string(id) + std::to_string(texture.source);
	OgreLog("Loading image " + name);

	const auto pixelFormat = [&]
	{
		if (image.component == 3)
			return Ogre::PF_B8G8R8;
		if (image.component == 4)
			return Ogre::PF_B8G8R8A8;

		//TODO do this properly. Right now it is guesswork

		OgreLog("unrecognized pixel format from tinygltf image");
	}();

	if (image.image.size() / image.component == image.width*image.height)
	{
		OgreLog("It looks like the image.component field and the image size does match");
	}
	else
	{
		OgreLog("I have no idea what is going on with the image format");
	}

	Ogre::Image OgreImage;
	OgreImage.loadDynamicImage(const_cast<Ogre::uchar*>(image.image.data()),
		image.width, image.height, 1, pixelFormat);

	Ogre::TexturePtr OgreTexture = textureManager->createManual(name,
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		Ogre::TextureType::TEX_TYPE_2D, image.width, image.height,
		1, 1, pixelFormat, Ogre::TU_DEFAULT
#ifdef DEBUG_TEXTURE_OUTPUT
		| Ogre::TU_RENDERTARGET
#endif
	);

	OgreTexture->loadImage(OgreImage);

#ifdef DEBUG_TEXTURE_OUTPUT
	//This was for debug, for that line to work you nee dthe texture to be declared with "texture usage render target"
	OgreTexture->getBuffer()->getRenderTarget()->writeContentsToTimestampedFile(name, ".png");
#endif

	loadedTextures.insert({ texture.source, OgreTexture });
}

Ogre_glTF_textureImporter::Ogre_glTF_textureImporter(tinygltf::Model& input) :
	model{ input }
{
	id++;
}

void Ogre_glTF_textureImporter::loadTextures()
{
	for (const auto& texture : model.textures)
	{
		loadTexture(texture);
	}
}

Ogre::TexturePtr Ogre_glTF_textureImporter::getTexture(int glTFTextureSourceID)
{
	auto texture = loadedTextures.find(glTFTextureSourceID);
	if (texture == std::end(loadedTextures)) return { };

	return texture->second;
}