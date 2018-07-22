#include "Ogre_glTF_common.hpp"

#include <OgreLogManager.h>

void OgreLog(const std::string& message)
{
#ifdef _DEBUG
	Ogre::LogManager::getSingleton().logMessage(message);
#else
	//Do something with message?
#endif
}

void OgreLog(const std::stringstream& message) { OgreLog(message.str()); }
