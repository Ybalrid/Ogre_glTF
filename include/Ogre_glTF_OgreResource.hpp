#pragma once

#include <OgreSharedPtr.h>
#include <OgreResourceManager.h>

#include "Ogre_glTF_DLL.hpp"

namespace Ogre_glTF
{
	class Ogre_glTF_EXPORT GlbFile : public Ogre::Resource
	{
		using byte = Ogre::uint8;
		std::vector<byte> data;

		void readFromStream(Ogre::DataStreamPtr& stream);

		inline static GlbFile* cast(void* gltfFileRawPtr);

	protected:
		void loadImpl() override;
		void unloadImpl() override;
		size_t calculateSize() const override;

	public:
		GlbFile(Ogre::ResourceManager* creator,
				 const Ogre::String& name,
				 Ogre::ResourceHandle handle,
				 const Ogre::String& group,
				 bool isManual						= false,
				 Ogre::ManualResourceLoader* loader = nullptr);

		virtual ~GlbFile();

		const byte* getData() const;

		size_t getSize() const override;
	};

	using GlbFilePtr = Ogre::SharedPtr<GlbFile>;

	class GlbFileManager : public Ogre::ResourceManager, public Ogre::Singleton<GlbFileManager>
	{
	protected:
		Ogre::Resource* createImpl(const Ogre::String& name, Ogre::ResourceHandle handle, const Ogre::String& group, bool isManual, Ogre::ManualResourceLoader* loader, const Ogre::NameValuePairList* createParams) override;

	public:
		GlbFileManager();
		virtual ~GlbFileManager();

		virtual GlbFilePtr load(const Ogre::String& name, const Ogre::String& group);

		static GlbFileManager& getSingleton();
		static GlbFileManager* getSingletonPtr();
	};

}