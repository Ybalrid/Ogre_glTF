#include "Ogre_glTF_modelConverter.hpp"
#include "OgreMesh2.h"
#include "OgreMeshManager2.h"
#include "OgreSubMesh2.h"

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

struct Ogre_glTF_vertexBufferPart
{
	std::unique_ptr<Ogre_glTF_geometryBuffer_base> buffer;
	Ogre::VertexElementType type;
	Ogre::VertexElementSemantic semantic;
	size_t vertexCount;
	size_t perVertex;
};

inline void log(const std::string messages)
{
	Ogre::LogManager::getSingleton().logMessage(messages);
}

Ogre_glTF_modelConverter::Ogre_glTF_modelConverter(tinygltf::Model& input) :
	model{ input }
{
}

Ogre::VertexBufferPackedVec constructVertexBuffer(const std::vector<Ogre_glTF_vertexBufferPart>& parts)
{
	Ogre::VertexElement2Vec vertexElements;
	size_t stride{ 0 };
	size_t count{ 0 };
	for (const auto& part : parts)
	{
		vertexElements.emplace_back(part.type, part.semantic);
		log("per vertex value : " + std::to_string(part.perVertex));
		stride += part.buffer->elementSize() * part.perVertex;
		log("In vertex buffer part there's " + std::to_string(part.vertexCount));
		count = part.vertexCount;
	}

	Ogre_glTF_geometryBuffer<unsigned char> finalBuffer(count * stride);
	for (size_t i = 0; i < count; ++i)
	{
		size_t written = 0;
		for (const auto& part : parts)
		{
			const size_t currentPartStride = part.buffer->elementSize();
			memcpy(&finalBuffer.data()[written + i * stride],
				&part.buffer->dataAddress()[i*currentPartStride],
				currentPartStride
			);
			written += part.buffer->elementSize();
		}
	}

	Ogre::VertexBufferPackedVec vec;
	auto vertexBuffer = Ogre_glTF_modelConverter::getVaoManager()->createVertexBuffer(vertexElements,
		count,
		Ogre::BT_DEFAULT,
		finalBuffer.data(),
		true);

	vec.push_back(vertexBuffer);
	return vec;
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

		std::vector<Ogre_glTF_vertexBufferPart> parts;
		log("\t primitive has : " + std::to_string(primitive.attributes.size()) + " atributes");
		for (const auto& atribute : primitive.attributes)
		{
			log(atribute.first);
			parts.push_back(std::move(extractVertexBuffer(atribute)));
		}

		auto vertexBuffers = constructVertexBuffer(parts);
		auto vao = getVaoManager()->createVertexArrayObject(vertexBuffers, indexBuffer, [&]()
		{
			switch (primitive.mode)
			{
			case TINYGLTF_MODE_LINE:
				return Ogre::OT_LINE_LIST;
			case TINYGLTF_MODE_LINE_LOOP:
				return Ogre::OT_LINE_STRIP;
			case TINYGLTF_MODE_POINTS:
				return Ogre::OT_POINT_LIST;
			case TINYGLTF_MODE_TRIANGLES:
				return Ogre::OT_TRIANGLE_LIST;
			case TINYGLTF_MODE_TRIANGLE_FAN:
				return Ogre::OT_TRIANGLE_FAN;
			case TINYGLTF_MODE_TRIANGLE_STRIP:
				return Ogre::OT_TRIANGLE_STRIP;
			default:
				throw std::runtime_error("Can't understand primitive mode!");
			};
		}());

		subMesh->mVao[Ogre::VpNormal].push_back(vao);
		subMesh->mVao[Ogre::VpShadow].push_back(vao);
	}

	//TODO fix that
	//Set the bounds to get frustum culling and LOD to work correctly.
	OgreMesh->_setBounds(Ogre::Aabb(Ogre::Vector3::ZERO, Ogre::Vector3::UNIT_SCALE), false);
	OgreMesh->_setBoundingSphereRadius(1.732f);

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

Ogre::IndexBufferPacked* Ogre_glTF_modelConverter::extractIndexBuffer(int accessorID) const
{
	auto& accessor = model.accessors[accessorID];
	auto& bufferView = model.bufferViews[accessor.bufferView];
	auto& buffer = model.buffers[bufferView.buffer];

	const auto indexBufferByteLen = bufferView.byteLength;
	size_t indexBufferLen;
	std::unique_ptr<Ogre_glTF_geometryBuffer_base> geometryBuffer{ nullptr };
	Ogre::IndexBufferPacked::IndexType type;

	switch (accessor.componentType)
	{
	case TINYGLTF_COMPONENT_TYPE_SHORT:;
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
		indexBufferLen = indexBufferByteLen / sizeof(Ogre::uint16);
		geometryBuffer = std::make_unique<Ogre_glTF_geometryBuffer<Ogre::uint16>>(indexBufferLen);
		type = Ogre::IndexBufferPacked::IT_16BIT;
		break;
	case TINYGLTF_COMPONENT_TYPE_INT:;
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
		indexBufferLen = indexBufferByteLen / sizeof(Ogre::uint32);
		type = Ogre::IndexBufferPacked::IT_32BIT;
		geometryBuffer = std::make_unique<Ogre_glTF_geometryBuffer<Ogre::uint32>>(indexBufferLen);
		break;
	default:
		throw std::runtime_error("Unrecognized index data format");
	}

	const auto chunkLen = geometryBuffer->elementSize();
	for (size_t i = 0; i < indexBufferByteLen; i += chunkLen)
	{
		memcpy(&geometryBuffer->dataAddress()[i],
			&buffer.data[(bufferView.byteOffset + accessor.byteOffset) + i * bufferView.byteStride],
			chunkLen);
	}

	return getVaoManager()->createIndexBuffer(type, indexBufferLen, Ogre::BT_IMMUTABLE, geometryBuffer->dataAddress(), true);
}

size_t getVertexElementBufferCount(int type)
{
	switch (type)
	{
	case TINYGLTF_TYPE_MAT2: return 2 * 2;
	case TINYGLTF_TYPE_MAT3: return 3 * 3;
	case TINYGLTF_TYPE_MAT4: return 4 * 4;
	case TINYGLTF_TYPE_VEC2: return 2;
	case TINYGLTF_TYPE_VEC3: return 3;
	case TINYGLTF_TYPE_VEC4: return 4;
	default: return 0;
	}
}

Ogre::VertexElementSemantic getVertexElementScemantic(const std::string& type)
{
	if (type == "POSITION") return Ogre::VES_POSITION;
	if (type == "NORMAL") return Ogre::VES_NORMAL;
	if (type == "TANGENT") return Ogre::VES_TANGENT;
	if (type == "TEXTCOORD_0") return Ogre::VES_TEXTURE_COORDINATES;
	if (type == "TEXTCOORD_1") return Ogre::VES_TEXTURE_COORDINATES;
	if (type == "COLOR_0") return Ogre::VES_DIFFUSE;
}

Ogre_glTF_vertexBufferPart Ogre_glTF_modelConverter::extractVertexBuffer(const std::pair<std::string, int>& attribute) const
{
	const auto elementScemantic = getVertexElementScemantic(attribute.first);
	const auto& accessor = model.accessors[attribute.second];
	const auto& bufferView = model.bufferViews[accessor.bufferView];
	const auto& buffer = model.buffers[bufferView.buffer];
	const auto vertexBufferByteLen = bufferView.byteLength;
	const auto count = getVertexElementBufferCount(accessor.type);
	size_t vertexBufferLen{ 0 };

	std::unique_ptr<Ogre_glTF_geometryBuffer_base> geometryBuffer{ nullptr };

	bool doublePrecision{ false };
	switch (accessor.componentType)
	{
	case TINYGLTF_COMPONENT_TYPE_DOUBLE:
		doublePrecision = true;
	case TINYGLTF_COMPONENT_TYPE_FLOAT:
		vertexBufferLen = (vertexBufferByteLen / sizeof(Ogre::Real));
		geometryBuffer = std::make_unique<Ogre_glTF_geometryBuffer<Ogre::Real>>(vertexBufferLen);
		break;

	default:
		throw std::runtime_error("Unrecognized vertex buffer coponent type");
	}

	Ogre::VertexElementType elementType;
	if (count == 2) elementType = Ogre::VET_FLOAT2;
	if (count == 3) elementType = Ogre::VET_FLOAT3;
	if (count == 4) elementType = Ogre::VET_FLOAT4;

	if (!doublePrecision)
	{
		const auto chunkLen = count * geometryBuffer->elementSize();
		for (size_t i = 0; i < vertexBufferByteLen; i += chunkLen)
		{
			memcpy(&geometryBuffer->dataAddress()[i],
				&buffer.data[bufferView.byteOffset + accessor.byteOffset + i * bufferView.byteStride],
				chunkLen);
		}
	}
	else
	{
		log("Attempting to load a double precision model. Sorry but as implemented, this is unsuported.");
		log("Using narrowing conversion to load the model");
		for (size_t i = 0; i < vertexBufferLen; i++)
		{
			*(reinterpret_cast<float*>(&geometryBuffer->dataAddress()[i * geometryBuffer->elementSize()])) =
				float(*(reinterpret_cast<double*>(buffer.data[bufferView.byteOffset + accessor.byteOffset
					+ i * bufferView.byteStride])));
		}
	}

	return {
		std::move(geometryBuffer),
		elementType,
		elementScemantic,
		vertexBufferLen / count,
		count
	};
}