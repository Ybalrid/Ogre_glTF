#pragma once

#include "Ogre_glTF_DLL.hpp"
#include "Ogre_glTF.hpp"

#include <OgrePlugin.h>
#include <memory>

namespace Ogre_glTF
{
	struct  gltfPluginAccessor
	{
		virtual ~gltfPluginAccessor() = default;

		///Get you a gltf loader that you can use to get Items made out of glTF meshes
		virtual glTFLoaderInterface* getLoader() const = 0;

		///This to seach throught the list of installed pluigins, finds the gltf one, and up-cast it to a gltfAccessor pointer for you to use
		static gltfPluginAccessor* findPlugin()
		{
			const auto list						  = Ogre::Root::getSingleton().getInstalledPlugins();
			const auto thisPluginInstanceIterator = std::find_if(std::begin(list), std::end(list), [](const Ogre::Plugin* plugPtr) {
				static const Ogre::String name = "Ogre glTF Loader";
				return plugPtr->getName() == name;
			});

			if(thisPluginInstanceIterator != std::end(list)) return dynamic_cast<gltfPluginAccessor*>(*thisPluginInstanceIterator);

			return nullptr;
		}
	};

	class Ogre_glTF_EXPORT glTFLoaderPlugin final : public Ogre::Plugin, public gltfPluginAccessor
	{
		Ogre::String name							= "Ogre glTF Loader";
		std::unique_ptr<Ogre_glTF::glTFLoader> gltf = nullptr;

	public:
		glTFLoaderPlugin();
		virtual ~glTFLoaderPlugin();
		const Ogre::String& getName() const override;
		void install() override;
		void initialise() override;
		void shutdown() override;
		void uninstall() override;


		glTFLoaderInterface* getLoader() const override
		{
			return gltf.get();
		}
	};
}
