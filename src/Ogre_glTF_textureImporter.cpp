#include "Ogre_glTF_textureImporter.hpp"
#include "Ogre_glTF_common.hpp"
#include <OgreLogManager.h>
#include <OgreColourValue.h>
#include <OgreRoot.h>

#include "OgreConfigFile.h"
#include <OgreMemoryAllocatorConfig.h>
#include "OgreTextureFilters.h"
#include "Ogre_glTF.hpp"
#include "OgrePrerequisites.h"


using namespace Ogre_glTF;

size_t textureImporter::mId { 0 };

textureImporter::textureImporter(tinygltf::Model& input) : mModel { input } { 
	mId++; 
	const auto renderSystem	= Ogre::Root::getSingleton().getRenderSystem();
	mTextureManager = renderSystem->getTextureGpuManager();
}

void textureImporter::preparePixelBuffer(Ogre::uint32 componentOffset, const tinygltf::Image* sourceImage)
{
	int index;
	int newSize = sourceImage->width * sourceImage->height * 4; 
	mPixelBuffer.resize(newSize);
	auto sourceOffset = sourceImage->image.data() + componentOffset;
	for(auto y = 0; y < sourceImage->height; ++y)
	{
		for(auto x = 0; x < sourceImage->width; ++x)
		{
			index		   = (x + y * sourceImage->width) * 4;
			mPixelBuffer[index] = sourceOffset[index];
		}
	}
}

Ogre::TextureGpu* textureImporter::getTexture(
	int glTFTextureIndex,
	Ogre::PbsTextureTypes texType,
	Ogre::PixelFormatGpu inputPixelFormat)
{

	const auto& image = mModel.images[glTFTextureIndex];
	Ogre::uchar* imageData;
	Ogre::uint32 filters = Ogre::TextureFilter::TypeGenerateDefaultMipmaps;
	Ogre::String texTypeasString;

	switch(texType){
		case Ogre::PbsTextureTypes::PBSM_NORMAL: 
			filters        |= Ogre::TextureFilter::TypePrepareForNormalMapping;
			texTypeasString = "NORMAL";
			imageData		= const_cast<Ogre::uchar*>(image.image.data());
			break;
		case Ogre::PbsTextureTypes::PBSM_DIFFUSE: 
			texTypeasString = "DIFFUSE";
			imageData		= const_cast<Ogre::uchar*>(image.image.data());
			break;
		case Ogre::PbsTextureTypes::PBSM_EMISSIVE: 
			texTypeasString = "EMISSIVE";
			imageData		= const_cast<Ogre::uchar*>(image.image.data());
			break;
		case Ogre::PbsTextureTypes::PBSM_ROUGHNESS:
			filters |= Ogre::TextureFilter::TypeLeaveChannelR;
			texTypeasString = "ROUGHNESS";
			preparePixelBuffer(1, &image);
			imageData		= mPixelBuffer.data();
			break;
		case Ogre::PbsTextureTypes::PBSM_METALLIC:
			filters |= Ogre::TextureFilter::TypeLeaveChannelR;
			texTypeasString = "METALLIC";
			preparePixelBuffer(2, &image);
			imageData		= mPixelBuffer.data();
			break;
	}

	const auto name = "glTF_texture_" + image.name + "_" + texTypeasString + "_" + std::to_string(glTFTextureIndex);

	auto texture = mTextureManager->findTextureNoThrow(name);
	if(texture)
	{
		OgreLog("texture: '" + name + "' Already loaded in Ogre::TextureGpuManager");
		return texture;
	}
	OgreLog("Can't find texure '" + name + "'. Generating it from glTF");
	
	Ogre::TextureGpu* ogreTexture;
	ogreTexture = mTextureManager->createOrRetrieveTexture(
		name,
		Ogre::GpuPageOutStrategy::Discard, Ogre::TextureFlags::ManualTexture | Ogre::TextureFlags::AutomaticBatching,
		Ogre::TextureTypes::Type2D,
		Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
	    filters);

	ogreTexture->setResolution(image.width, image.height);
	ogreTexture->setPixelFormat(inputPixelFormat);
	ogreTexture->setNumMipmaps(Ogre::PixelFormatGpuUtils::getMaxMipmapCount(image.width, image.height, 1));

	ogreTexture->_transitionTo(Ogre::GpuResidency::Resident, nullptr);
	ogreTexture->_setNextResidencyStatus(Ogre::GpuResidency::Resident);

	auto sizeInBytes = Ogre::PixelFormatGpuUtils::calculateSizeBytes(image.width, image.height, 1, 1, inputPixelFormat, 1, 4);
	auto buffer = reinterpret_cast<std::uint8_t*>(OGRE_MALLOC_SIMD(sizeInBytes, Ogre::MEMCATEGORY_RESOURCE));

	Ogre::Image2 ogreImage;
	ogreImage.loadDynamicImage(buffer, image.width, image.height, 1, Ogre::TextureTypes::Type2D, inputPixelFormat, true, 1);

	std::memcpy(buffer, imageData, sizeInBytes);
	ogreImage.generateMipmaps(ogreTexture->prefersLoadingFromFileAsSRGB(), Ogre::Image2::FILTER_GAUSSIAN_HIGH);
	ogreImage.uploadTo(ogreTexture, 0, ogreImage.getNumMipmaps() - 1);
	return ogreTexture;
}

