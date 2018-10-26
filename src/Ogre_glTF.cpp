#include <utility>

#include "Ogre_glTF.hpp"
#include "Ogre_glTF_modelConverter.hpp"
#include "Ogre_glTF_textureImporter.hpp"
#include "Ogre_glTF_materialLoader.hpp"
#include "Ogre_glTF_skeletonImporter.hpp"
#include "Ogre_glTF_common.hpp"

#include "Ogre_glTF_OgreResource.hpp"

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
	impl() : textureImp(model), materialLoad(model, textureImp), modelConv(model), skeletonImp(model) {}

	///Variable to check if everything is alright with the adapter
	bool valid = false;

	///The model object that data will be loaded into and read from
	tinygltf::Model model;

	///Where tinygltf will write it's error status
	std::string error = "";

	///Where tinygltf will write it's warning messages
	std::string warnings = "";

	///Texture importer object : go through the texture array and load them into Ogre
	textureImporter textureImp;

	///Material loader : get the data from the material section of the glTF file and create an HlmsDatablock to use
	materialLoader materialLoad;

	///Model converter : load all the actual mesh data from the glTF file, and convert them into index and vertex buffer that can
	///be used to create an Ogre VAO (Vertex Array Object), then create a mesh for it
	modelConverter modelConv;

	///Skeleton importer : load skins from the glTF model, create equivalent OgreSkeleton objects
	skeletonImporter skeletonImp;
};

loaderAdapter::loaderAdapter() : pimpl{ std::make_unique<impl>() }
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
		pimpl->textureImp.loadTextures();
		Ogre::MeshPtr Mesh = getMesh();

		auto Item = smgr->createItem(Mesh);
		for(size_t i = 0; i < Item->getNumSubItems(); ++i)
		{
			Item->getSubItem(i)->setDatablock(getDatablock(i));
		}
		return Item;
	}
	return nullptr;
}

void loaderAdapter::getTransforms(ItemAndTransforms* tran)
{
	this->pimpl->modelConv.getTransforms(tran);
}

Ogre::MeshPtr loaderAdapter::getMesh() const
{
	auto Mesh = this->pimpl->modelConv.getOgreMesh();

	if(this->pimpl->modelConv.hasSkins())
	{
		//load skeleton information
		auto skeleton = this->pimpl->skeletonImp.getSkeleton(this->adapterName);
		Mesh->_notifySkeleton(skeleton);
	}
	return Mesh;
}

Ogre::HlmsDatablock* loaderAdapter::getDatablock(size_t index) const { return pimpl->materialLoad.getDatablock(index); }

loaderAdapter::loaderAdapter(loaderAdapter&& other) noexcept : pimpl{ std::move(other.pimpl) }
{
	//OgreLog("Moved adapter object...");
}

loaderAdapter& loaderAdapter::operator=(loaderAdapter&& other) noexcept
{
	pimpl = std::move(other.pimpl);
	return *this;
}

bool loaderAdapter::isOk() const { return pimpl->valid; }

std::string loaderAdapter::getLastError() const { return pimpl->error; }

///Implementation of the glTF loader. Exist as a pImpl inside the glTFLoader class
struct glTFLoader::glTFLoaderImpl
{
	///The loader object from TinyGLTF
	tinygltf::TinyGLTF loader;

	///Constructor. the loader is on the stack, there isn't much state to set inside the object
	glTFLoaderImpl() { OgreLog("initialized TinyGLTF loader"); }

	///For file type detection. Ascii is plain old JSON text, Binary is .glc files.
	enum class FileType { Ascii, Binary, Unknown };

	///Probe inside the file, or check the extension to determine if we have to load a text file, or a binary file
	FileType detectType(const std::string& path) const
	{
		//Quickly open the file as binary and check if there's the gltf binary magic number
		{
			auto probe = std::ifstream(path, std::ios_base::binary);
			if(!probe) throw std::runtime_error("Could not open " + path);

			std::array<char, 5> buffer{};
			for(size_t i{ 0 }; i < 4; ++i) probe >> buffer[i];
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
			case FileType::Unknown: return false;
			case FileType::Ascii:
				//OgreLog("Detected ascii file type");
				return loader.LoadASCIIFromFile(&adapter.pimpl->model, &adapter.pimpl->error, &adapter.pimpl->warnings, path);
			case FileType::Binary:
				//OgreLog("Deteted binary file type");
				return loader.LoadBinaryFromFile(&adapter.pimpl->model, &adapter.pimpl->error, &adapter.pimpl->warnings, path);
		}
	}

	bool loadGlb(loaderAdapter& adapter, GlbFilePtr file)
	{
		return loader.LoadBinaryFromMemory(
			&adapter.pimpl->model, &adapter.pimpl->error, &adapter.pimpl->warnings, file->getData(), int(file->getSize()), ".", 0);
	}
};

glTFLoader::glTFLoader() : loaderImpl{ std::make_unique<glTFLoaderImpl>() }
{
	if(Ogre::Root::getSingletonPtr() == nullptr) throw std::runtime_error("Please create an Ogre::Root instance before initializing the glTF library!");

	if(!Ogre_glTF::GlbFileManager::getSingletonPtr()) new GlbFileManager;

	OgreLog("glTFLoader created!");
}

loaderAdapter glTFLoader::loadFromFileSystem(const std::string& path) const
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

	adapter.pimpl->modelConv.debugDump();
	return adapter;
}

loaderAdapter glTFLoader::loadGlbResource(const std::string& name) const
{
	OgreLog("Loading GLB from resource manager " + name);
	auto& glbManager = GlbFileManager::getSingleton();
	auto glbFile	 = glbManager.load(name, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);

	loaderAdapter adapter;
	if(glbFile)
	{
		loaderImpl->loadGlb(adapter, glbFile);
		adapter.pimpl->valid = true;
	}

	adapter.pimpl->modelConv.debugDump();
	return adapter;
}

Ogre::Item* glTFLoader::getItemFromResource(const std::string& name, Ogre::SceneManager* smgr)
{
	OgreLog("Getting resource");
	auto adapter = loadGlbResource(name);
	if(adapter.isOk()) { OgreLog("Adapter is ok!"); }
	else
	{
		OgreLog("Adapter is not okay!");
	}

	OgreLog("Calling get item with your smgr...");
	return adapter.getItem(smgr);
}

ItemAndTransforms glTFLoader::getItemAndTransformsFromResource(const std::string& name, Ogre::SceneManager* smgr)
{
	ItemAndTransforms data{ nullptr, Ogre::Vector3::ZERO, Ogre::Vector3::UNIT_SCALE, Ogre::Quaternion::IDENTITY };
	OgreLog("Getting resource");
	auto adapter = loadGlbResource(name);
	if(adapter.isOk()) { OgreLog("Adapter is ok!"); }
	else
	{
		OgreLog("Adapter is not okay!");
	}
	OgreLog("Calling get item with your smgr...");
	data.item = adapter.getItem(smgr);
	adapter.getTransforms(&data);

	return { data};
}

Ogre::Item* glTFLoader::getItemFromFileSystem(const std::string& fileName, Ogre::SceneManager* smgr)
{
	auto adapter = loadFromFileSystem(fileName);
	if(adapter.isOk()) { OgreLog("Adapter is ok!"); }
	else
	{
		OgreLog("Adapter is not okay!");
	}
	return adapter.getItem(smgr);
}

MeshAndDataBlock glTFLoader::getMeshFromFileSystem(const std::string& name)
{
	auto adapter = loadFromFileSystem(name);
	if(adapter.isOk()) { OgreLog("Adapter is ok!"); }
	else
	{
		OgreLog("Adapter is not okay!");
	}
	return { adapter.getMesh(), adapter.getDatablock(0) };
}

MeshAndDataBlock glTFLoader::getMeshFromResource(const std::string& name)
{
	auto adapter = loadGlbResource(name);
	if(adapter.isOk()) { OgreLog("Adapter is ok!"); }
	else
	{
		OgreLog("Adapter is not okay!");
	}
	return { adapter.getMesh(), adapter.getDatablock(0) };
}

glTFLoader::glTFLoader(glTFLoader&& other) noexcept : loaderImpl(std::move(other.loaderImpl)) {}

glTFLoader& glTFLoader::operator=(glTFLoader&& other) noexcept
{
	loaderImpl = std::move(other.loaderImpl);
	return *this;
}

glTFLoader::~glTFLoader() = default;
