#include "Ogre_glTF.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "tiny_gltf.h"

struct Ogre_glTF_adapter::impl
{
	bool valid;
	tinygltf::Model model;
	tinygltf::Material material;
};

Ogre_glTF_adapter::Ogre_glTF_adapter() :
	pimpl{ std::make_unique<Ogre_glTF_adapter::impl>() }
{
}

Ogre_glTF_adapter::~Ogre_glTF_adapter()
{
}

Ogre::Item* Ogre_glTF_adapter::getItem() const
{
	return nullptr;
}

Ogre_glTF_adapter::Ogre_glTF_adapter(Ogre_glTF_adapter&& other) noexcept : pimpl{ std::move(other.pimpl) }
{
}

bool Ogre_glTF_adapter::isOk() const
{
	return pimpl->valid;
}

Ogre_glTF::Ogre_glTF()
{
	if (Ogre::Root::getSingletonPtr() == nullptr)
		throw std::runtime_error("Please create an Ogre::Root instance before initializing the glTF library!");

	Ogre::LogManager::getSingleton().logMessage("Ogre_glTF created!");
}

Ogre_glTF_adapter Ogre_glTF::loadFile(const std::string& path) const
{
	Ogre_glTF_adapter adapter;

	return std::move(adapter);
}