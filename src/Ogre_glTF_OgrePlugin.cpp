#include "Ogre_glTF_OgrePlugin.hpp"

Ogre_glTF::glTFLoaderPlugin* gPluginInstaller = nullptr;

extern "C" {

void Ogre_glTF_EXPORT dllStartPlugin(void)
{
	if(gPluginInstaller)
	{
		throw std::runtime_error("Apparently called dllStartPlugin on the Ogre_glTF"
								 "plugin twice. I'm afraid you can't do that... ");
	}

	gPluginInstaller = new Ogre_glTF::glTFLoaderPlugin;
	Ogre::Root::getSingleton().installPlugin(gPluginInstaller);
}

void Ogre_glTF_EXPORT dllStopPlugin(void)
{
	Ogre::Root::getSingleton().uninstallPlugin(gPluginInstaller);
	delete gPluginInstaller;
	gPluginInstaller = nullptr;
}
}

Ogre_glTF::glTFLoaderPlugin::glTFLoaderPlugin() : Ogre::Plugin() {}

Ogre_glTF::glTFLoaderPlugin::~glTFLoaderPlugin() = default;

const Ogre::String& Ogre_glTF::glTFLoaderPlugin::getName() const { return name; }

void Ogre_glTF::glTFLoaderPlugin::install() {}

void Ogre_glTF::glTFLoaderPlugin::initialise() { gltf = std::make_unique<Ogre_glTF::glTFLoader>(); }

void Ogre_glTF::glTFLoaderPlugin::shutdown() { gltf = nullptr; }

void Ogre_glTF::glTFLoaderPlugin::uninstall() {}
