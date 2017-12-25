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

class Ogre_glTF_EXPORT Ogre_glTF
{
public:
	Ogre_glTF();
};