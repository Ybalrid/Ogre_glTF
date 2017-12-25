#include "Ogre_glTF.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "tiny_gltf.h"

Ogre_glTF::Ogre_glTF()
{
	if (Ogre::Root::getSingletonPtr() == nullptr) 
		throw std::runtime_error("Please create an Ogre::Root instance before initializing the glTF library!");

	Ogre::LogManager::getSingleton().logMessage("Ogre_glTF created!");
}