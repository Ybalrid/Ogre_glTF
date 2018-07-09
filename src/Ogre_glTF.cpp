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

using namespace Ogre_glTF;

///Implementaiton of the adapter
struct loaderAdapter::impl
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
	textureImporter textureImporter;

	///Mateiral loader : get the data from the material section of the glTF file and create an HlmsDatablock to use
	materialLoader materialLoader;

	///Model converter : load all the actual mesh data from the glTF file, and convert them into index and vertex buffer that can
	///be used to create an Ogre VAO (Vertex Array Object), then create a mesh for it
	modelConverter modelConverter;

	///Skeleton importer : load skins from the glTF model, create equivalent OgreSkeleton objects
	skeletonImporter skeletonImporter;
};

loaderAdapter::loaderAdapter() :
 pimpl { std::make_unique<impl>() }
{
	//OgreLog("Created adapter object...");
}

loaderAdapter::~loaderAdapter()
{
	//OgreLog("Destructed adapter object...");
}

Ogre::Item* loaderAdapter::getItem(Ogre::SceneManager* smgr) const
{
	if(isOk())
	{
		pimpl->textureImporter.loadTextures();
		auto Mesh = pimpl->modelConverter.getOgreMesh();
		if(pimpl->modelConverter.hasSkins())
		{
			//load skeleton information
			auto skeleton = pimpl->skeletonImporter.getSkeleton(adapterName);
			Mesh->_notifySkeleton(skeleton);
		}

		auto Item = smgr->createItem(Mesh);
		Item->setDatablock(pimpl->materialLoader.getDatablock());
		return Item;
	}
	return nullptr;
}

loaderAdapter::loaderAdapter(loaderAdapter&& other) noexcept :
 pimpl { std::move(other.pimpl) }
{
	//OgreLog("Moved adapter object...");
}

loaderAdapter& loaderAdapter::operator=(loaderAdapter&& other) noexcept
{
	pimpl = std::move(other.pimpl);
	return *this;
}

bool loaderAdapter::isOk() const
{
	return pimpl->valid;
}

std::string loaderAdapter::getLastError() const
{
	return pimpl->error;
}

///Implementation of the glTF loader. Exist as a pImpl inside the glTFLoader class
struct glTFLoader::glTFLoaderImpl
{
	///The loader object from TinyGLTF
	tinygltf::TinyGLTF loader;

	///Construtor. the loader is on the stack, there isn't mutch state to set inside the object
	glTFLoaderImpl()
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
				//OgreLog("Detected binary file thanks to the magic number at the start!");
				return FileType::Binary;
			}
		}

		//If we don't have any better, check the file extension.
		auto extension = path.substr(path.find_last_of('.') + 1);
		std::transform(std::begin(extension), std::end(extension), std::begin(extension), [](char c) { return char(tolower(int(c))); });
		if(extension == "gltf") return FileType::Ascii;
		if(extension == "glb") return FileType::Binary;

		return FileType::Unknown;
	}

	///Load the content of a file into an adapter object
	bool loadInto(loaderAdapter& adapter, const std::string& path)
	{
		switch(detectType(path))
		{
			default:
			case FileType::Unknown:
				return false;
			case FileType::Ascii:
				//OgreLog("Detected ascii file type");
				return loader.LoadASCIIFromFile(&adapter.pimpl->model, &adapter.pimpl->error, path);
			case FileType::Binary:
				//OgreLog("Deteted binary file type");
				return loader.LoadBinaryFromFile(&adapter.pimpl->model, &adapter.pimpl->error, path);
		}
	}
};

glTFLoader::glTFLoader() :
 loaderImpl { std::make_unique<glTFLoaderImpl>() }
{
	if(Ogre::Root::getSingletonPtr() == nullptr)
		throw std::runtime_error("Please create an Ogre::Root instance before initializing the glTF library!");

	OgreLog("glTFLoader created!");
}

loaderAdapter glTFLoader::loadFile(const std::string& path) const
{
	OgreLog("loading file " + path);
	loaderAdapter adapter;
	adapter.adapterName = path;
	loaderImpl->loadInto(adapter, path);
	//if (adapter.getLastError().empty())
	{
		OgreLog("Debug : it looks like the file was loaded without error!");
		adapter.pimpl->valid = true;
	}

	adapter.pimpl->modelConverter.debugDump();
	return std::move(adapter);
}

glTFLoader::glTFLoader(glTFLoader&& other) noexcept :
 loaderImpl(std::move(other.loaderImpl))
{
}

glTFLoader& glTFLoader::operator=(glTFLoader&& other) noexcept
{
	loaderImpl = std::move(other.loaderImpl);
	return *this;
}

glTFLoader::~glTFLoader() = default;
