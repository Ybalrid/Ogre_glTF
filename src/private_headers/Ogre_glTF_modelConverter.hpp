#pragma once
#include <Ogre.h>
#include <tiny_gltf.h>

#include <Ogre_glTF_DLL.hpp>

class Ogre_glTF_geometryBuffer_base
{
public:
	virtual ~Ogre_glTF_geometryBuffer_base() = default;
	virtual unsigned char* dataAddress() = 0;
	virtual size_t dataSize() const = 0;
	virtual size_t elementSize() const = 0;
	virtual void debugContentToLog() const = 0;
};

template <typename T>
class Ogre_glTF_geometryBuffer : public Ogre_glTF_geometryBuffer_base
{
	T* buffer;
	const size_t bufferSize;

	static T* allocateSimdBuffer(size_t size)
	{
		return reinterpret_cast<T*>(OGRE_MALLOC_SIMD(sizeof(T) * size, Ogre::MEMCATEGORY_GEOMETRY));
	}

	static void freeSimdBuffer(T* buffer)
	{
		OGRE_FREE_SIMD(buffer, Ogre::MEMCATEGORY_GEOMETRY);
	}

public:

	void debugContentToLog() const final
	{
		for (size_t i{ 0 }; i < bufferSize; ++i)
		{
			Ogre::LogManager::getSingleton().logMessage("BufferContent[" + std::to_string(i) + "] = " + std::to_string(buffer[i]));
		}
	}

	unsigned char* dataAddress() final { return reinterpret_cast<unsigned char*>(data()); }
	size_t dataSize() const final { return bufferSize; }
	size_t elementSize() const final { return sizeof(T); }

	Ogre_glTF_geometryBuffer(size_t size) :
		buffer{ allocateSimdBuffer(size) },
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
	{
	}
};

struct Ogre_glTF_vertexBufferPart
{
	std::unique_ptr<Ogre_glTF_geometryBuffer_base> buffer;
	Ogre::VertexElementType type;
	Ogre::VertexElementSemantic semantic;
	size_t vertexCount;
	size_t perVertex;

	size_t getPartStride() const;
};

template <typename bufferType, typename sourceType> void loadIndexBuffer(bufferType* dest,
	sourceType* source,
	size_t indexCount,
	size_t offset,
	size_t stride)
{
	for (size_t i = 0; i < indexCount; ++i)
	{
		dest[i] = *(reinterpret_cast<sourceType*>(reinterpret_cast<unsigned char*>(source) + (offset + i * stride)));
	}
}

class Ogre_glTF_EXPORT Ogre_glTF_modelConverter
{
public:
	Ogre_glTF_modelConverter(tinygltf::Model& intput);
	Ogre::MeshPtr generateOgreMesh();
	void debugDump() const;
private:
	static Ogre::VaoManager* getVaoManager();
	static size_t getVertexBufferElementsPerVertexCount(int type);
	static Ogre::VertexElementSemantic getVertexElementScemantic(const std::string& type);

	Ogre::IndexBufferPacked* extractIndexBuffer(int accessor) const;
	Ogre_glTF_vertexBufferPart extractVertexBuffer(const std::pair<std::string, int>& attribute) const;
	Ogre::VertexBufferPackedVec constructVertexBuffer(const std::vector<Ogre_glTF_vertexBufferPart>& parts) const;

	tinygltf::Model& model;
};