#include "Ogre_glTF.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "tiny_gltf.h"

inline void log(const std::string& message)
{
	Ogre::LogManager::getSingleton().logMessage(message);
}

struct Ogre_glTF_adapter::impl
{
	bool valid = false;
	tinygltf::Model model;
	std::string error;
};

Ogre_glTF_adapter::Ogre_glTF_adapter() :
	pimpl{ std::make_unique<Ogre_glTF_adapter::impl>() }
{
	log("Created adapter object...");
}

Ogre_glTF_adapter::~Ogre_glTF_adapter()
{
	log("Destructed adapter object...");
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

std::string Ogre_glTF_adapter::getLastError() const
{
	return pimpl->error;
}

struct Ogre_glTF::gltfLoader
{
	tinygltf::TinyGLTF loader;

	gltfLoader()
	{
		log("initialized TinyGLTF loader");
	}

	enum class FileType
	{
		Ascii,
		Binary,
		Unknown
	};

	FileType detectType(const std::string& path)
	{
		//Quickly open the file as binary and chekc if there's the gltf binary magic number
		{
			auto probe = std::ifstream(path, std::ios_base::binary);
			if (!probe) throw std::runtime_error("Could not open " + path);
			std::array<char, 5> buffer;
			for (size_t i{ 0 }; i < 4; ++i)
				probe >> buffer[i];
			buffer[4] = 0;
			if (std::string("gltf") == std::string(buffer.data()))
				return FileType::Binary;
		}

		//If we don't have any better, check the file extension.
		auto extension = path.substr(path.find_last_of('.') + 1);
		std::transform(std::begin(extension), std::end(extension), std::begin(extension), ::tolower);
		if (extension == "gltf") return FileType::Ascii;
		if (extension == "glb") return FileType::Binary;

		return FileType::Unknown;
	}

	bool loadInto(Ogre_glTF_adapter& adapter, const std::string& path)
	{
		switch (detectType(path))
		{
		default:
		case FileType::Unknown:
			return false;
		case FileType::Ascii:
			log("Detected ascii file type");
			return loader.LoadASCIIFromFile(&adapter.pimpl->model, &adapter.pimpl->error, path);
		case FileType::Binary:
			log("Deteted binary file type");
			return loader.LoadBinaryFromFile(&adapter.pimpl->model, &adapter.pimpl->error, path);
		}
	}
};

Ogre_glTF::Ogre_glTF() : loaderImpl{ std::make_unique<Ogre_glTF::gltfLoader>() }
{
	if (Ogre::Root::getSingletonPtr() == nullptr)
		throw std::runtime_error("Please create an Ogre::Root instance before initializing the glTF library!");

	log("Ogre_glTF created!");
}

Ogre_glTF::~Ogre_glTF()
{
}

Ogre_glTF_adapter Ogre_glTF::loadFile(const std::string& path) const
{
	log("Attempting to log " + path);
	Ogre_glTF_adapter adapter;
	loaderImpl->loadInto(adapter, path);
	return std::move(adapter);
}

Ogre_glTF::Ogre_glTF(Ogre_glTF&& other) noexcept :
loaderImpl(std::move(other.loaderImpl))
{
}