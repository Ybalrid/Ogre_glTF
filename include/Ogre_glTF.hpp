#pragma once

#include <Ogre.h>

#ifdef _WIN32
#ifdef DLLDIR_EX
#define Ogre_glTF_EXPORT __declspec(dllexport)
#else
#define Ogre_glTF_EXPORT __declspec(dllimport)
#endif
#else
#define Ogre_glTF_EXPORT //dummy
#endif

class Ogre_glTF;

class Ogre_glTF_EXPORT Ogre_glTF_adapter
{
	friend class Ogre_glTF;
	Ogre_glTF_adapter();
public:
	Ogre::Item* getItem();
};

class Ogre_glTF_EXPORT Ogre_glTF
{
public:
	Ogre_glTF();
	//non copiable
	Ogre_glTF(const Ogre_glTF&) = delete;
	Ogre_glTF& operator=(const Ogre_glTF&) = delete;

	Ogre_glTF_adapter loadFile(const std::string& path);

private:
};