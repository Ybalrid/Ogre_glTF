#include "Ogre_glTF_modelConverter.hpp"

inline void log(const std::string messages)
{
	Ogre::LogManager::getSingleton().logMessage(messages);
}

Ogre_glTF_modelConverter::Ogre_glTF_modelConverter(tinygltf::Model& input) :
	model{ input }
{
}

void Ogre_glTF_modelConverter::debugDump()
{
	std::stringstream ss;

	ss << "This glTF model has:\n"
		<< model.accessors.size() << " accessors\n"
		<< model.animations.size() << " animations\n"
		<< model.buffers.size() << " buffers\n"
		<< model.bufferViews.size() << " bufferViews\n"
		<< model.materials.size() << " materials\n"
		<< model.meshes.size() << " meshes\n"
		<< model.nodes.size() << " nodes\n"
		<< model.textures.size() << " textures\n"
		<< model.images.size() << " images\n"
		<< model.skins.size() << " skins\n"
		<< model.samplers.size() << " samplers\n"
		<< model.cameras.size() << " cameras\n"
		<< model.scenes.size() << " scenes\n"
		<< model.lights.size() << " lights\n"
		;

	log(ss.str());
}