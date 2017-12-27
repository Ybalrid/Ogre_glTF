#include "Ogre_glTF_modelConverter.hpp"
#include "OgreMesh2.h"
#include "OgreMeshManager2.h"
#include "OgreSubMesh2.h"

inline void log(const std::string messages)
{
	Ogre::LogManager::getSingleton().logMessage(messages);
}

Ogre_glTF_modelConverter::Ogre_glTF_modelConverter(tinygltf::Model& input) :
	model{ input }
{
}

Ogre::MeshPtr Ogre_glTF_modelConverter::generateOgreMesh()
{
	const auto mainMeshIndex = model.nodes[model.scenes[model.defaultScene].nodes.front()].mesh;
	const auto& mesh = model.meshes[mainMeshIndex];
	log("Found mesh " + mesh.name);
	log("mesh has " + std::to_string(mesh.primitives.size()) + " primitives");

	auto OgreMesh = Ogre::MeshManager::getSingleton().createManual(mesh.name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	for (const auto& primitive : mesh.primitives)
	{
		auto subMesh = OgreMesh->createSubMesh();
		auto indexBuffer = extractIndexBuffer(primitive.indices);

		//TODO load vertex buffer below :

		log("\t primitive has : " + std::to_string(primitive.attributes.size()) + " atributes");
		for (const auto& atribute : primitive.attributes)
		{
			log(atribute.first);
		}
	}

	return OgreMesh;
}

void Ogre_glTF_modelConverter::debugDump()
{
	std::stringstream ss;

	ss << "This glTF model has:\n"
		<< model.accessors.size() << " accessors\n"
		<< model.animations.size() << " animations\n"
		<< model.buffers.size() << " buffers\n"
		<< model.bufferViews.size() << " bufferViews\n"
		<< model.materials.size() << " materials\n"
		<< model.meshes.size() << " meshes\n"
		<< model.nodes.size() << " nodes\n"
		<< model.textures.size() << " textures\n"
		<< model.images.size() << " images\n"
		<< model.skins.size() << " skins\n"
		<< model.samplers.size() << " samplers\n"
		<< model.cameras.size() << " cameras\n"
		<< model.scenes.size() << " scenes\n"
		<< model.lights.size() << " lights\n"
		;

	log(ss.str());
}

Ogre::VaoManager* Ogre_glTF_modelConverter::getVaoManager()
{
	//Our class shouldn't be able to exist if Ogre hasn't been initalized. This call should allways succeed.
	return Ogre::Root::getSingletonPtr()->getRenderSystem()->getVaoManager();
}

class Ogre_glTF_geometryBuffer_base
{
public:
	virtual ~Ogre_glTF_geometryBuffer_base() = default;
	virtual unsigned char* dataAddress() = 0;
	virtual size_t dataSize() = 0;
	virtual size_t elementSize() = 0;
};

template <typename T> class Ogre_glTF_geometryBuffer : public Ogre_glTF_geometryBuffer_base
{
	T* buffer;
	const size_t bufferSize;

	static T* createSimdBuffer(size_t size)
	{
		return reinterpret_cast<T*>(OGRE_MALLOC_SIMD(sizeof(T) * size, Ogre::MEMCATEGORY_GEOMETRY));
	}

	static void freeSimdBuffer(T* buffer)
	{
		OGRE_FREE_SIMD(buffer, Ogre::MEMCATEGORY_GEOMETRY);
	}

public:

	unsigned char* dataAddress() final { return reinterpret_cast<unsigned char*>(data()); }
	size_t dataSize() final { return bufferSize; }
	size_t elementSize() final { return sizeof(T); }

	Ogre_glTF_geometryBuffer(size_t size) :
		buffer{ createSimdBuffer(size) },
		bufferSize{ size }
	{
	}

	~Ogre_glTF_geometryBuffer()
	{
		freeSimdBuffer(buffer);
	}

	T* data() { return buffer; }

	size_t size() const { return bufferSize; }

	Ogre_glTF_geometryBuffer(const Ogre_glTF_geometryBuffer&) = delete;
	Ogre_glTF_geometryBuffer operator=(const Ogre_glTF_geometryBuffer&) = delete;
	Ogre_glTF_geometryBuffer(Ogre_glTF_geometryBuffer&& other) noexcept :
	buffer{ other.buffer },
		bufferSize{ other.bufferSize }
	{}
};

Ogre::IndexBufferPacked* Ogre_glTF_modelConverter::extractIndexBuffer(int accessorID) const
{
	auto vaoManager{ getVaoManager() };

	auto& accessor = model.accessors[accessorID];
	auto& bufferView = model.bufferViews[accessor.bufferView];
	auto& buffer = model.buffers[bufferView.buffer];

	auto indexBufferLen = bufferView.byteLength;

	std::unique_ptr<Ogre_glTF_geometryBuffer_base> geometryBuffer{ nullptr };
	Ogre::IndexBufferPacked::IndexType type;
	switch (accessor.componentType)
	{
	case TINYGLTF_COMPONENT_TYPE_SHORT:
		geometryBuffer = std::make_unique<Ogre_glTF_geometryBuffer<Ogre::int16>>(indexBufferLen);
		type = Ogre::IndexBufferPacked::IT_16BIT;
		break;
	case TINYGLTF_COMPONENT_TYPE_INT:
		geometryBuffer = std::make_unique<Ogre_glTF_geometryBuffer<Ogre::int32>>(indexBufferLen);
		type = Ogre::IndexBufferPacked::IT_32BIT;
		break;
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
		geometryBuffer = std::make_unique<Ogre_glTF_geometryBuffer<Ogre::uint16>>(indexBufferLen);
		type = Ogre::IndexBufferPacked::IT_16BIT;
		break;
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
		type = Ogre::IndexBufferPacked::IT_32BIT;
		geometryBuffer = std::make_unique<Ogre_glTF_geometryBuffer<Ogre::uint32>>(indexBufferLen);
		break;
	default:
		throw std::runtime_error("Unrecognized index data format");
	}

	const auto nbIndices = indexBufferLen / geometryBuffer->elementSize();
	for (size_t i = 0; i < nbIndices; i++)
	{
		memcpy(&geometryBuffer->dataAddress()[i],
			&buffer.data[(bufferView.byteOffset + accessor.byteOffset) + i * bufferView.byteStride],
			geometryBuffer->elementSize());
	}

	return vaoManager->createIndexBuffer(type, nbIndices, Ogre::BT_IMMUTABLE, geometryBuffer->dataAddress(), true);
}

Ogre::VertexBufferPacked* Ogre_glTF_modelConverter::extractVertexBuffer(int accessor) const
{
	auto vaoManager{ getVaoManager() };

	return nullptr;
}