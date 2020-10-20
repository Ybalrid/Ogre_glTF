#include "Ogre_glTF_materialLoader.hpp"
#include "Ogre_glTF_textureImporter.hpp"
#include "Ogre_glTF_common.hpp"
#include <OgreHlmsPbsDatablock.h>
#include <OgreHlms.h>
#include <OgreHlmsManager.h>
#include <OgreLogManager.h>
#include "Ogre_glTF_internal_utils.hpp"

using namespace Ogre_glTF;

materialLoader::materialLoader(tinygltf::Model& input, textureImporter& textureInterface) : 
	textureImporterRef { textureInterface }, 
	model { input } 
{}

Ogre::Vector3 materialLoader::convertColor(const tinygltf::ColorValue& color)
{
	std::array<Ogre::Real, 4> colorBuffer{};
	internal_utils::container_double_to_real(color, colorBuffer);
	return Ogre::Vector3 { colorBuffer.data() };
}

void materialLoader::setBaseColor(Ogre::HlmsPbsDatablock* block, Ogre::Vector3 color) const
{
	block->setDiffuse(color);
}

void materialLoader::setMetallicValue(Ogre::HlmsPbsDatablock* block, Ogre::Real value) const
{
	block->setMetalness(value);
}

void materialLoader::setRoughnesValue(Ogre::HlmsPbsDatablock* block, Ogre::Real value) const
{
	block->setRoughness(value);
}

void materialLoader::setEmissiveColor(Ogre::HlmsPbsDatablock* block, Ogre::Vector3 color) const
{
	block->setEmissive(color);
}

bool materialLoader::isTextureIndexValid(int value) const
{
	return !(value < 0);
}

void materialLoader::setBaseColorTexture(Ogre::HlmsPbsDatablock* block, int value) const
{
	if(!isTextureIndexValid(value)) return;
	auto texture = textureImporterRef.getTexture(value, Ogre::PbsTextureTypes::PBSM_DIFFUSE);
	if(texture)
		block->setTexture(Ogre::PbsTextureTypes::PBSM_DIFFUSE, texture);
}

void materialLoader::setMetalRoughTexture(Ogre::HlmsPbsDatablock* block, int gltfTextureID) const
{
	if(!isTextureIndexValid(gltfTextureID)) return;
	//Ogre cannot use combined metal rough textures. Metal is in the R channel, and rough in the G channel. 
	//It seems that the images are loaded as BGR by the libarry
	//R channel is channle 2 (from 0), G channel is 1.

	auto metalTexure = textureImporterRef.getTexture(gltfTextureID, Ogre::PBSM_METALLIC, Ogre::PixelFormatGpu::PFG_RGBA8_UNORM);
	auto roughTexure = textureImporterRef.getTexture(gltfTextureID, Ogre::PBSM_ROUGHNESS, Ogre::PixelFormatGpu::PFG_RGBA8_UNORM);
	
	if(metalTexure)
		block->setTexture(Ogre::PBSM_METALLIC, metalTexure);

	if(roughTexure)
		block->setTexture(Ogre::PBSM_ROUGHNESS, roughTexure);
}

void materialLoader::setNormalTexture(Ogre::HlmsPbsDatablock* block, int value) const
{
	if(!isTextureIndexValid(value)) return;
	auto texture = textureImporterRef.getTexture(value, Ogre::PbsTextureTypes::PBSM_NORMAL);
	//auto texture = textureImporterRef.getTexture(value);
	if(texture)
	{
		block->setTexture(Ogre::PbsTextureTypes::PBSM_NORMAL, texture);
	}
}

void materialLoader::setOcclusionTexture(Ogre::HlmsPbsDatablock* block, int value) const
{
	if(!isTextureIndexValid(value)) return;
	auto texture = textureImporterRef.getTexture(value, Ogre::PbsTextureTypes::PBSM_DIFFUSE);
	if(texture)
	{
		//OgreLog("occlusion texture from textureImporter : " + texture->getName());
		//OgreLog("Warning: Ogre doesn't supoort occlusion map in it's HLMS PBS implementation!");
		//block->setTexture(Ogre::PbsTextureTypes::PBSM_DIFFUSE, 0, texture);
	}
}

void materialLoader::setEmissiveTexture(Ogre::HlmsPbsDatablock* block, int value) const
{
	if(!isTextureIndexValid(value)) return;
	auto texture = textureImporterRef.getTexture(value, Ogre::PbsTextureTypes::PBSM_EMISSIVE);
	if(texture)
	{
		//OgreLog("emissive texture from textureImporter : " + texture->getName());
		block->setTexture(Ogre::PbsTextureTypes::PBSM_EMISSIVE, texture);
	}
}

void materialLoader::setAlphaMode(Ogre::HlmsPbsDatablock* block, const std::string& mode) const
{
	if(mode == "BLEND")
	{
		auto blendBlock = *block->getBlendblock();
		blendBlock.setBlendType(Ogre::SBT_TRANSPARENT_ALPHA);
		block->setBlendblock(blendBlock);
	}
	else if(mode =="MASK")
	{
		block->setAlphaTest( Ogre::CMPF_GREATER_EQUAL );
	}
}

void materialLoader::setAlphaCutoff(Ogre::HlmsPbsDatablock* block, Ogre::Real value) const
{
	block->setAlphaTestThreshold(value);
}

Ogre::HlmsDatablock* materialLoader::getDatablock(size_t index) const
{
	OgreLog("Loading material...");
	auto HlmsPbs			 = static_cast<Ogre::HlmsPbs*>(Ogre::Root::getSingleton().getHlmsManager()->getHlms(Ogre::HlmsTypes::HLMS_PBS));
	const auto material		 = model.materials[index];

	auto datablock = static_cast<Ogre::HlmsPbsDatablock*>(HlmsPbs->getDatablock(Ogre::IdString(material.name)));
	
	if(datablock){
		OgreLog("Found HlmsPbsDatablock " + material.name + " in Ogre::HlmsPbs");
		return datablock;
	}

	datablock = static_cast<Ogre::HlmsPbsDatablock*>(HlmsPbs->createDatablock(
		Ogre::IdString(material.name),
		material.name,
		Ogre::HlmsMacroblock {},
		Ogre::HlmsBlendblock {},
		Ogre::HlmsParamVec {}));

	datablock->setWorkflow(Ogre::HlmsPbsDatablock::Workflows::MetallicWorkflow);

	for(const auto& content : material.values) 
		handleMaterialValue(datablock, content.first, &content.second);	
	
	for(const auto& content : material.additionalValues) 
		handleMaterialValue(datablock, content.first, &content.second);	

	return datablock;
}

void materialLoader::handleMaterialValue(Ogre::HlmsPbsDatablock* dataBlock, std::string key,const tinygltf::Parameter* param) const
{ 
	if (key == "baseColorTexture") { 
		setBaseColorTexture(dataBlock, param->TextureIndex());
	} else if (key == "metallicRoughnessTexture"){
		setMetalRoughTexture(dataBlock, param->TextureIndex());
	} else if(key == "normalTexture"){
		setNormalTexture(dataBlock, param->TextureIndex());
	} else if(key == "emissiveTexture"){
		setEmissiveTexture(dataBlock, param->TextureIndex());
	} else if(key == "baseColorFactor"){
		setBaseColor(dataBlock, convertColor(param->ColorFactor()));
		// Need to set the alpha channel separately
		float alpha			 = float(param->number_array[3]);
		auto transparentMode = (alpha == 1) ? Ogre::HlmsPbsDatablock::None : Ogre::HlmsPbsDatablock::Transparent;
		dataBlock->setTransparency(alpha, transparentMode);
	} else if(key == "metallicFactor"){
		setMetallicValue(dataBlock, static_cast<float>(param->Factor()));
	} else if(key == "roughnessFactor"){
		setRoughnesValue(dataBlock, static_cast<float>(param->Factor()));
	} else if(key == "emissiveFactor"){
		setEmissiveColor(dataBlock, convertColor(param->ColorFactor()));
	} else if(key == "alphaMode"){
		setAlphaMode(dataBlock, param->string_value);
	} else if(key == "alphaCutoff"){
		setAlphaCutoff(dataBlock, static_cast<Ogre::Real>(param->number_value));
	} else {
		OgreLog("Ogre_glTF unhandled material param: '" + key + "'");
	}
}

size_t materialLoader::getDatablockCount() const //todo this could use some refactoring. This information is actually fetched like, twice.
{
	const auto mainMeshIndex = (model.defaultScene != 0 ? model.nodes[model.scenes[model.defaultScene].nodes.front()].mesh : 0);
	const auto& mesh = model.meshes[mainMeshIndex];
	return mesh.primitives.size();
}
