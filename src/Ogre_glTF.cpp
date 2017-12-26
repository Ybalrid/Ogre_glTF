#include "Ogre_glTF.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "tiny_gltf.h"

Ogre_glTF_adapter::Ogre_glTF_adapter()
{
}

Ogre::Item* Ogre_glTF_adapter::getItem()
{
	return nullptr;
}

Ogre_glTF::Ogre_glTF()
{
	if (Ogre::Root::getSingletonPtr() == nullptr)
		throw std::runtime_error("Please create an Ogre::Root instance before initializing the glTF library!");

	Ogre::LogManager::getSingleton().logMessage("Ogre_glTF created!");
}

Ogre_glTF_adapter Ogre_glTF::loadFile(const std::string& path)
{
	//TODO implement me
	return Ogre_glTF_adapter();
}