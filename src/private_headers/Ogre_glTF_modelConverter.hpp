#pragma once
#include <Ogre.h>
#include <tiny_gltf.h>

#include <Ogre_glTF_DLL.hpp>

class Ogre_glTF_EXPORT Ogre_glTF_modelConverter
{
public:
	Ogre_glTF_modelConverter(tinygltf::Model& intput);
	Ogre::MeshPtr generateOgreMesh();
	void debugDump();
private:

	static Ogre::VaoManager* getVaoManager();
	Ogre::IndexBufferPacked* extractIndexBuffer(int accessor);
	Ogre::VertexBufferPacked* extractVertexBuffer();

	tinygltf::Model& model;
};