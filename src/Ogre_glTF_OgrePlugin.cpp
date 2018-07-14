#include "Ogre_glTF_OgrePlugin.hpp"
#include <algorithm>
Ogre_glTF::glTFLoaderPlugin::glTFLoaderPlugin() : Ogre::Plugin() {}

Ogre_glTF::glTFLoaderPlugin::~glTFLoaderPlugin() = default;

const Ogre::String& Ogre_glTF::glTFLoaderPlugin::getName() const { return name; }

void Ogre_glTF::glTFLoaderPlugin::install() {}

void Ogre_glTF::glTFLoaderPlugin::initialise() { gltf = std::make_unique<Ogre_glTF::glTFLoader>(); }

void Ogre_glTF::glTFLoaderPlugin::shutdown() { gltf = nullptr; }

void Ogre_glTF::glTFLoaderPlugin::uninstall() {}

Ogre_glTF::glTFLoaderPlugin* Ogre_glTF::glTFLoaderPlugin::getGltfPluginInstance()
{
	const auto list						  = Ogre::Root::getSingleton().getInstalledPlugins();
	const auto thisPluginInstanceIterator = std::find_if(std::begin(list), std::end(list), [](const Ogre::Plugin* plugPtr) {
		static const Ogre::String name = "Ogre glTF Loader";
		return plugPtr->getName() == name;
	});

	if(thisPluginInstanceIterator != std::end(list)) return dynamic_cast<glTFLoaderPlugin*>(*thisPluginInstanceIterator);

	return nullptr;
}

Ogre_glTF::glTFLoader* Ogre_glTF::glTFLoaderPlugin::getGlTFLoader() const { return gltf.get(); }
