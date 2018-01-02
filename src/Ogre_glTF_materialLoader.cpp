#include "Ogre_glTF_materialLoader.hpp"
#include "Ogre_glTF_textureImporter.hpp"
#include <OgreHlmsPbsDatablock.h>
#include <OgreHlms.h>
#include <OgreHlmsManager.h>
#include <OgreLogManager.h>

inline void OgreLog(const std::string& message)
{
	Ogre::LogManager::getSingleton().logMessage(message);
}

bool Ogre_glTF_materialLoader::isTextureIndexValid(int value) const
{
	return !(value < 0);
}

void Ogre_glTF_materialLoader::setBaseColorTexture(Ogre::HlmsPbsDatablock* block, int value) const
{
	if (!isTextureIndexValid(value)) return;
	auto texture = textureImporter.getTexture(value);
	if (texture)
	{
		OgreLog("diffuse texture from textureImporter : " + texture->getName());
		block->setTexture(Ogre::PbsTextureTypes::PBSM_DIFFUSE, 0, texture);
	}
}

void Ogre_glTF_materialLoader::setMetalRoughTexture(Ogre::HlmsPbsDatablock* block, int gltfTextureID) const
{
	if (!isTextureIndexValid(gltfTextureID)) return;
	//Ogre cannot use combined metal rough textures. Metal is in the R channel, and rough in the G channel. It seems that the images are loaded as BGR by the libarry
	//R channel is channle 2 (from 0), G channel is 1.

	auto metalTexure = textureImporter.generateGreyScaleFromChannel(gltfTextureID, 2);
	auto roughTexure = textureImporter.generateGreyScaleFromChannel(gltfTextureID, 1);

	if (metalTexure)
	{
		OgreLog("metalness greyscale texture extracted by textureImporter : " + metalTexure->getName());
		block->setTexture(Ogre::PBSM_METALLIC, 0, metalTexure);
	}

	if (roughTexure)
	{
		OgreLog("roughness geyscale texture extracted by textureImporter : " + roughTexure->getName());
		block->setTexture(Ogre::PBSM_ROUGHNESS, 0, roughTexure);
	}
}

void Ogre_glTF_materialLoader::setNormalTexture(Ogre::HlmsPbsDatablock* block, int value) const
{
	if (!isTextureIndexValid(value)) return;
	auto texture = textureImporter.getTexture(value);
	if (texture)
	{
		OgreLog("normal texture from textureImporter : " + texture->getName());
		block->setTexture(Ogre::PbsTextureTypes::PBSM_NORMAL, 0, texture);
	}
}

void Ogre_glTF_materialLoader::setOcclusionTexture(Ogre::HlmsPbsDatablock* block, int value) const
{
	if (!isTextureIndexValid(value)) return;
	auto texture = textureImporter.getTexture(value);
	if (texture)
	{
		OgreLog("occlusion texture from textureImporter : " + texture->getName());
		OgreLog("Warning: Ogre doesn't supoort occlusion map in it's HLMS PBS implementation!");
		//block->setTexture(Ogre::PbsTextureTypes::PBSM_, 0, texture);
	}
}

Ogre_glTF_materialLoader::Ogre_glTF_materialLoader(tinygltf::Model& input, Ogre_glTF_textureImporter& textureInterface) :
	textureImporter{ textureInterface },
	model{ input }
{
}

Ogre::HlmsDatablock* Ogre_glTF_materialLoader::getDatablock() const
{
	OgreLog("Loading material...");
	//TODO this will need some modification if we support multiple meshes by glTF file
	auto HlmsPbs = static_cast<Ogre::HlmsPbs*>(Ogre::Root::getSingleton().getHlmsManager()->getHlms(Ogre::HlmsTypes::HLMS_PBS));
	const auto mainMeshIndex = model.nodes[model.scenes[model.defaultScene].nodes.front()].mesh;
	const auto& mesh = model.meshes[mainMeshIndex];
	const auto material = model.materials[mesh.primitives.front().material];

	auto block = static_cast<Ogre::HlmsPbsDatablock*>(HlmsPbs->createDatablock(Ogre::IdString(material.name),
		material.name, Ogre::HlmsMacroblock{}, Ogre::HlmsBlendblock{}, Ogre::HlmsParamVec{}));
	block->setWorkflow(Ogre::HlmsPbsDatablock::Workflows::MetallicWorkflow);

	//TODO refactor these almost exact peices of code
	OgreLog("values");
	for (const auto& content : material.values)
	{
		OgreLog(content.first);
		if (content.first == "baseColorTexture")
			setBaseColorTexture(block, getTextureIndex(content));

		if (content.first == "metallicRoughnessTexture")
			setMetalRoughTexture(block, getTextureIndex(content));
	}

	OgreLog("additionalValues");
	for (const auto& content : material.additionalValues)
	{
		OgreLog(content.first);
		if (content.first == "normalTexture")
			setMetalRoughTexture(block, getTextureIndex(content));

		if (content.first == "occlusionTexture")
			setMetalRoughTexture(block, getTextureIndex(content));
	}

	OgreLog("extCommonValues");
	for (const auto& content : material.extCommonValues)
		OgreLog(content.first);

	OgreLog("extPBRValues");
	for (const auto& content : material.extPBRValues)
		OgreLog(content.first);

	return block;
}