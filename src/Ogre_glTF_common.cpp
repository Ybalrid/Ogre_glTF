#include "Ogre_glTF_common.hpp"

#include <OgreLogManager.h>

void OgreLog(const std::string& message) { Ogre::LogManager::getSingleton().logMessage(message); }

void OgreLog(const std::stringstream& message) { OgreLog(message.str()); }
