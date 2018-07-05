#include <utility>

#include "Ogre_glTF.hpp"
#include "Ogre_glTF_modelConverter.hpp"
#include "Ogre_glTF_textureImporter.hpp"
#include "Ogre_glTF_materialLoader.hpp"
#include "Ogre_glTF_skeletonImporter.hpp"
#include "Ogre_glTF_common.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

#include <OgreItem.h>
#include <OgreMesh2.h>
#include <OgreSubMesh2.h>
#include <OgreSkeleton.h>

///Implementaiton of the adapter
struct Ogre_glTF_adapter::impl
{
	///Constructor, initialize once all the objects inclosed in this class. They need a reference
	///to a model object (and sometimes more) given at construct time
	impl() :
	 textureImporter(model),
	 materialLoader(model, textureImporter),
	 modelConverter(model),
	 skeletonImporter(model)
	{}

	///Vaiable to check if everything is allright with the adapter
	bool valid = false;

	///The model object that data will be loaded into and read from
	tinygltf::Model model;
	///Where tinygltf will write it's error status
	std::string error = "";

	///Texture importer object : go throught the texture array and load them into Ogre
	Ogre_glTF_textureImporter textureImporter;
	///Mateiral loader : get the data from the material section of the glTF file and create an HlmsDatablock to use
	Ogre_glTF_materialLoader materialLoader;
	///Model converter : load all the actual mesh data from the glTF file, and convert them into index and vertex buffer that can
	///be used to create an Ogre VAO (Vertex Array Object), then create a mesh for it
	Ogre_glTF_modelConverter modelConverter;
	///Skeleton importer : load skins from the glTF model, create equivalent OgreSkeleton objects
	Ogre_glTF_skeletonImporter skeletonImporter;
};

Ogre_glTF_adapter::Ogre_glTF_adapter() :
 pimpl { std::make_unique<Ogre_glTF_adapter::impl>() }
{
	OgreLog("Created adapter object...");
}

Ogre_glTF_adapter::~Ogre_glTF_adapter()
{
	OgreLog("Destructed adapter object...");
}

Ogre::Item* Ogre_glTF_adapter::getItem(Ogre::SceneManager* smgr) const
{
	if(isOk())
	{
		pimpl->textureImporter.loadTextures();
		auto Mesh = pimpl->modelConverter.getOgreMesh();
		if(pimpl->modelConverter.hasSkins())
		{
			//load skeleton information
			auto skeleton = pimpl->skeletonImporter.getSkeleton();
			Mesh->_notifySkeleton(skeleton);
		}

		auto Item = smgr->createItem(Mesh);
		Item->setDatablock(pimpl->materialLoader.getDatablock());
		return Item;
	}
	return nullptr;
}

Ogre_glTF_adapter::Ogre_glTF_adapter(Ogre_glTF_adapter&& other) noexcept :
 pimpl { std::move(other.pimpl) }
{
	OgreLog("Moved adapter object...");
}

Ogre_glTF_adapter& Ogre_glTF_adapter::operator=(Ogre_glTF_adapter&& other) noexcept
{
	pimpl = std::move(other.pimpl);
	return *this;
}

bool Ogre_glTF_adapter::isOk() const
{
	return pimpl->valid;
}

std::string Ogre_glTF_adapter::getLastError() const
{
	return pimpl->error;
}

///Implementation of the glTF loader. Exist as a pImpl inside the Ogre_glTF class
struct Ogre_glTF::gltfLoader
{
	///The loader object from TinyGLTF
	tinygltf::TinyGLTF loader;

	///Construtor. the loader is on the stack, there isn't mutch state to set inside the object
	gltfLoader()
	{
		OgreLog("initialized TinyGLTF loader");
	}

	///For file type detection. Ascii is plain old JSON text, Binary is .glc files.
	enum class FileType {
		Ascii,
		Binary,
		Unknown
	};

	///Probe inside the file, or check the extension to determine if we have to load a text file, or a binary file
	FileType detectType(const std::string& path) const
	{
		//Quickly open the file as binary and chekc if there's the gltf binary magic number
		{
			auto probe = std::ifstream(path, std::ios_base::binary);
			if(!probe)
				throw std::runtime_error("Could not open " + path);

			std::array<char, 5> buffer {};
			for(size_t i { 0 }; i < 4; ++i)
				probe >> buffer[i];
			buffer[4] = 0;

			if(std::string("glTF") == std::string(buffer.data()))
			{
				OgreLog("Detected binary file thanks to the magic number at the start!");
				return FileType::Binary;
			}
		}

		//If we don't have any better, check the file extension.
		auto extension = path.substr(path.find_last_of('.') + 1);
		std::transform(std::begin(extension), std::end(extension), std::begin(extension), [](char c) { return char(::tolower(int(c))); });
		if(extension == "gltf") return FileType::Ascii;
		if(extension == "glb") return FileType::Binary;

		return FileType::Unknown;
	}

	///Load the content of a file into an adapter object
	bool loadInto(Ogre_glTF_adapter& adapter, const std::string& path)
	{
		switch(detectType(path))
		{
			default:
			case FileType::Unknown:
				return false;
			case FileType::Ascii:
				OgreLog("Detected ascii file type");
				return loader.LoadASCIIFromFile(&adapter.pimpl->model, &adapter.pimpl->error, path);
			case FileType::Binary:
				OgreLog("Deteted binary file type");
				return loader.LoadBinaryFromFile(&adapter.pimpl->model, &adapter.pimpl->error, path);
		}
	}
};

Ogre_glTF::Ogre_glTF() :
 loaderImpl { std::make_unique<Ogre_glTF::gltfLoader>() }
{
	if(Ogre::Root::getSingletonPtr() == nullptr)
		throw std::runtime_error("Please create an Ogre::Root instance before initializing the glTF library!");

	OgreLog("Ogre_glTF created!");
}

Ogre_glTF::~Ogre_glTF()
	= default;

Ogre_glTF_adapter Ogre_glTF::loadFile(const std::string& path) const
{
	OgreLog("Attempting to log " + path);
	Ogre_glTF_adapter adapter;
	loaderImpl->loadInto(adapter, path);
	//if (adapter.getLastError().empty())
	{
		OgreLog("Debug : it looks like the file was loaded without error!");
		adapter.pimpl->valid = true;
	}

	adapter.pimpl->modelConverter.debugDump();
	return std::move(adapter);
}

Ogre_glTF::Ogre_glTF(Ogre_glTF&& other) noexcept :
 loaderImpl(std::move(other.loaderImpl))
{
}
