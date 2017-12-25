#include "Ogre_glTF.hpp"

Ogre_glTF::Ogre_glTF()
{
	if (Ogre::Root::getSingletonPtr() == nullptr) 
		throw std::runtime_error("Please create an Ogre::Root instance before initializing the glTF library!");

	Ogre::LogManager::getSingleton().logMessage("Ogre_glTF created!");
}