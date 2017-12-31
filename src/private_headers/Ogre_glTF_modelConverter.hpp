#pragma once
#include <Ogre.h>
#include <tiny_gltf.h>

#include <Ogre_glTF_DLL.hpp>

///Base class of all geometry buffers
class Ogre_glTF_geometryBuffer_base
{
public:
	virtual ~Ogre_glTF_geometryBuffer_base() = default;
	virtual unsigned char* dataAddress() = 0;
	virtual size_t dataSize() const = 0;
	virtual size_t elementSize() const = 0;
	virtual void debugContentToLog() const = 0;
};

///RAII style object that encapsulate a buffer allocated via OGRE_MALLOC_SIMD
///Any of theses buffer should be used with Ogre VaoManager **WITHOUT** shadow copy
template <typename T>
class Ogre_glTF_geometryBuffer : public Ogre_glTF_geometryBuffer_base
{
	T* buffer;
	const size_t bufferSize;

	///Call OGRE_MALLOC_SIMD and re-cast the output pointer to type T*
	static T* allocateSimdBuffer(size_t size)
	{
		return reinterpret_cast<T*>(OGRE_MALLOC_SIMD(sizeof(T) * size, Ogre::MEMCATEGORY_GEOMETRY));
	}

	///Free the buffer with OGRE_FREE_SIMD
	static void freeSimdBuffer(T* buffer)
	{
		OGRE_FREE_SIMD(buffer, Ogre::MEMCATEGORY_GEOMETRY);
	}

public:

	///Write the content of the buffer to the log. This is an expensinve operation only needed when debugging the loading code itself
	void debugContentToLog() const final
	{
		for (size_t i{ 0 }; i < bufferSize; ++i)
		{
			Ogre::LogManager::getSingleton().logMessage("BufferContent[" + std::to_string(i) + "] = " + std::to_string(buffer[i]));
		}
	}

	///Return a pointer to the start of the data. Pointer doesn't carry the type information of said data. Usefull as the basic location to memcpy data in and out this buffer
	unsigned char* dataAddress() final { return reinterpret_cast<unsigned char*>(data()); }
	///Return the lengh of the data as it was requested (in number of element)
	size_t dataSize() const final { return size(); }

	///Return the number of bytes an element is made from
	size_t elementSize() const final { return sizeof(T); }

	///Construct a geometryBuffer. This is templated and you will need to provide the type between angle brackets.
	Ogre_glTF_geometryBuffer(size_t size) :
		buffer{ allocateSimdBuffer(size) },
		bufferSize{ size }
	{
	}

	///Call freeSimdBuffer on the enclosed buffer
	~Ogre_glTF_geometryBuffer()
	{
		freeSimdBuffer(buffer);
	}

	///Return a typed pointer to the data. Think of it as getting access to the array behind this buffer
	T* data() { return buffer; }

	///Get the size of the buffer
	size_t size() const { return bufferSize; }

	///Deleted copy constructor
	Ogre_glTF_geometryBuffer(const Ogre_glTF_geometryBuffer&) = delete;
	///Deleted assignment operator
	Ogre_glTF_geometryBuffer operator=(const Ogre_glTF_geometryBuffer&) = delete;

	///Move constructor that move around the underlying pointer and size
	Ogre_glTF_geometryBuffer(Ogre_glTF_geometryBuffer&& other) noexcept :
	buffer{ other.buffer }, bufferSize{ other.bufferSize } {}
};

///Part of the vertex buffer, containing a geometry buffer and the information about the type and number of vertex elements;
///We cachce these informations because we need to reorder the data into one single interlaved buffer for Ogre loading vertices into a single Vao
struct Ogre_glTF_vertexBufferPart
{
	std::unique_ptr<Ogre_glTF_geometryBuffer_base> buffer;
	Ogre::VertexElementType type;
	Ogre::VertexElementSemantic semantic;
	size_t vertexCount;
	size_t perVertex;

	size_t getPartStride() const;
};

///Index loader that will also convert between interger types.
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

///Converter objet : take a tinygltf model and encapsulate all the code necessary to extract mesh information
class Ogre_glTF_EXPORT Ogre_glTF_modelConverter
{
public:
	///Construct a modelConverter from a model
	Ogre_glTF_modelConverter(tinygltf::Model& intput);

	///Return a mesh generated from the data inside the gltf model. Currently look for the mesh attached on the first node of the default scene
	Ogre::MeshPtr generateOgreMesh();

	///Print out debug information on the model structure
	void debugDump() const;
private:
	///Get a pointer to the Ogre::VaoManager
	static Ogre::VaoManager* getVaoManager();
	///Get how many elements is needed for a glTF vertex type. eg "3" for a VEC3, "16" for a MAT4
	static size_t getVertexBufferElementsPerVertexCount(int type);

	///Get the equivalent Ogre VertexElementSemantic from the type defined as a string in the glTF file. Both texture coordinates return VES_TEXTURE_COORDINATES regardless if it's the first or second.
	static Ogre::VertexElementSemantic getVertexElementScemantic(const std::string& type);

	///Get a IntexBufferPacked object from the VaoManager for the current accessor. Accessor is found on the mesh object, and point to the buffer alongside some metadata
	Ogre::IndexBufferPacked* extractIndexBuffer(int accessor) const;
	///Extract the buffer content from the attibute of a primitive of a mesh
	Ogre_glTF_vertexBufferPart extractVertexBuffer(const std::pair<std::string, int>& attribute) const;
	///Construct an actual vertex bufffer from a list of vertex buffer parts
	Ogre::VertexBufferPackedVec constructVertexBuffer(const std::vector<Ogre_glTF_vertexBufferPart>& parts) const;

	///Reference to a loaded model
	tinygltf::Model& model;
};