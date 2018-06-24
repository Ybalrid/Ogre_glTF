#pragma once
#include <Ogre.h>
#include <tiny_gltf.h>

#include <Ogre_glTF_DLL.hpp>

///Base class of all geometry buffers
class Ogre_glTF_geometryBuffer_base
{
public:
	///Default polymorphic destructor
	virtual ~Ogre_glTF_geometryBuffer_base() = default;
	///Return a pointer to the array of bytes that contitute the geometry data
	virtual unsigned char* dataAddress() = 0;
	///Get the size of the data in elements
	virtual size_t dataSize() const = 0;
	///Get the size in bytes of an element
	virtual size_t elementSize() const = 0;
	///Print to Ogre's log the raw content of this buffer. This is super slow.
	///Don't do that unless you need to debug the library itself
	virtual void _debugContentToLog() const = 0;
};

///RAII style object that encapsulate a buffer allocated via OGRE_MALLOC_SIMD
///Any of theses buffer should be used with Ogre VaoManager **WITHOUT** shadow copy
template <typename T>
class Ogre_glTF_geometryBuffer : public Ogre_glTF_geometryBuffer_base
{
	///Pointer to the buffer allocated with OGRE_MALLOC_SIMD, in the "geometry" memory category
	T* buffer;
	///Size of the buffer, not in bytes, but in number of T contained
	const size_t bufferSize;

	///Call OGRE_MALLOC_SIMD and re-cast the output pointer to type T*
	/// \param size number of T that we are allocating in the buffer
	static T* allocateSimdBuffer(size_t size)
	{
		return reinterpret_cast<T*>(OGRE_MALLOC_SIMD(sizeof(T) * size, Ogre::MEMCATEGORY_GEOMETRY));
	}

	///Free the buffer with OGRE_FREE_SIMD
	/// \param buffer pointer to the buffer we are freeing
	static void freeSimdBuffer(T* buffer)
	{
		OGRE_FREE_SIMD(buffer, Ogre::MEMCATEGORY_GEOMETRY);
	}

public:
	///Write the content of the buffer to the log. This is an expensinve operation only needed when debugging the loading code itself
	void _debugContentToLog() const final
	{
		for(size_t i{ 0 }; i < bufferSize; ++i)
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
	/// \param size size of the buffer we are allocation (in nuber of elements, not bytes)
	Ogre_glTF_geometryBuffer(size_t size) :
	 buffer{ allocateSimdBuffer(size) },
	 bufferSize{ size }
	{}

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
	/// \param other buffer we are moving into this one
	Ogre_glTF_geometryBuffer(Ogre_glTF_geometryBuffer&& other) noexcept :
	 buffer{ other.buffer }, bufferSize{ other.bufferSize } {}
};

///Part of the vertex buffer, containing a geometry buffer and the information about the type and number of vertex elements;
///We cachce these informations because we need to reorder the data into one single interlaved buffer for Ogre loading vertices into a single Vao
struct Ogre_glTF_vertexBufferPart
{
	///Buffer that contain veticies
	std::unique_ptr<Ogre_glTF_geometryBuffer_base> buffer;
	///The type of vertex data (2 floats, 3 floats...)
	Ogre::VertexElementType type;
	///The semantic of the vertex data in this part of the buffer (normal, position, texture coordinates...)
	Ogre::VertexElementSemantic semantic;
	///Number of vertices on this buffer
	size_t vertexCount;
	///Number of "basic elements" that constitute a vertex (2x for a 2D vector, 3x for a 3D vector...)
	size_t perVertex;
	///Get the number of bytes for the data stride
	size_t getPartStride() const;
};

///Index loader that will also convert between interger types.
/// \param dest where to write the indexes to
/// \param source where to read the indexes from
/// \param indexCount number of indexes
/// \param offset where the indexes starts in the buffer
/// \param stride number of bytes between elements
template <typename bufferType, typename sourceType>
void loadIndexBuffer(bufferType* dest,
					 sourceType* source,
					 size_t indexCount,
					 size_t offset,
					 size_t stride)
{
	for(size_t i = 0; i < indexCount; ++i)
	{
		dest[i] = *(reinterpret_cast<sourceType*>(reinterpret_cast<unsigned char*>(source) + (offset + i * stride)));
	}
}

///Converter objet : take a tinygltf model and encapsulate all the code necessary to extract mesh information
class Ogre_glTF_modelConverter
{
public:
	///Construct a modelConverter from a model
	/// \param input model we are converting into an Ogre model
	Ogre_glTF_modelConverter(tinygltf::Model& input);

	///Return a mesh generated from the data inside the gltf model. Currently look for the mesh attached on the first node of the default scene
	Ogre::MeshPtr getOgreMesh();

	///Print out debug information on the model structure
	void debugDump() const;

	///Return true if the model defines skins. Skins are "vertex to bone" asignment for skeletal animation
	bool hasSkins() const;

private:
	///Get a pointer to the Ogre::VaoManager
	static Ogre::VaoManager* getVaoManager();
	///Get how many elements is needed for a glTF vertex type. eg "3" for a VEC3, "16" for a MAT4
	/// \param type glTF defined vertex element type
	static size_t getVertexBufferElementsPerVertexCount(int type);

	///Get the equivalent Ogre VertexElementSemantic from the type defined as a string in the glTF file. Both texture coordinates return VES_TEXTURE_COORDINATES regardless if it's the first or second.
	/// \param type the string that represent the type of the buffer
	static Ogre::VertexElementSemantic getVertexElementScemantic(const std::string& type);

	///Get a IntexBufferPacked object from the VaoManager for the current accessor. Accessor is found on the mesh object, and point to the buffer alongside some metadata
	/// \param accessor index of the accessor to the index buffer
	Ogre::IndexBufferPacked* extractIndexBuffer(int accessor) const;
	///Extract the buffer content from the attibute of a primitive of a mesh
	/// \param attribute the attribue of the mesh primitive we are loading
	Ogre_glTF_vertexBufferPart extractVertexBuffer(const std::pair<std::string, int>& attribute, Ogre::Aabb& boundingBox) const;
	///Construct an actual vertex bufffer from a list of vertex buffer parts
	/// \param parts list of Ogre_glTF_vertexBufferPart to load into the vertex buffer
	Ogre::VertexBufferPackedVec constructVertexBuffer(const std::vector<Ogre_glTF_vertexBufferPart>& parts) const;

	///Reference to a loaded model
	tinygltf::Model& model;
};
