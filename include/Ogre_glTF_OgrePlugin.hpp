#pragma once

#include "Ogre_glTF_DLL.hpp"
#include "Ogre_glTF.hpp"

#include <OgrePlugin.h>
#include <memory>


namespace Ogre_glTF
{
	class Ogre_glTF_EXPORT glTFLoaderPlugin final : public Ogre::Plugin
	{
		Ogre::String name = "Ogre glTF Loader";
		std::unique_ptr<Ogre_glTF::glTFLoader> gltf = nullptr;

	public:
		glTFLoaderPlugin();
		virtual ~glTFLoaderPlugin();
		const Ogre::String& getName() const override;
		void install() override;
		void initialise() override;
		void shutdown() override;
		void uninstall() override;

		glTFLoader* getGlTFLoader() const;

		static glTFLoaderPlugin* getGltfPluginInstance();
	};
}