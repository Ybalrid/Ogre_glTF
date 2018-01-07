#include "Ogre_glTF_modelConverter.hpp"
#include "Ogre_glTF_common.hpp"
#include <OgreMesh2.h>
#include <OgreMeshManager2.h>
#include <OgreSubMesh2.h>

size_t Ogre_glTF_vertexBufferPart::getPartStride() const
{
	return buffer->elementSize() * perVertex;
}

Ogre_glTF_modelConverter::Ogre_glTF_modelConverter(tinygltf::Model& input) :
	model{ input }
{
}

Ogre::VertexBufferPackedVec Ogre_glTF_modelConverter::constructVertexBuffer(const std::vector<Ogre_glTF_vertexBufferPart>& parts) const
{
	Ogre::VertexElement2Vec vertexElements;
	size_t stride{ 0 }, strideInElements{ 0 };
	size_t vertexCount{ 0 }, previousVertexCount{ 0 };
	for (const auto& part : parts)
	{
		vertexElements.emplace_back(part.type, part.semantic);
		strideInElements += part.perVertex;
		stride += part.buffer->elementSize() * part.perVertex;
		vertexCount = part.vertexCount;

		//Sanity check
		if (previousVertexCount != 0)
		{
			if (vertexCount == previousVertexCount)
			{
				previousVertexCount = vertexCount;
			}
			else
			{
				throw std::runtime_error("Part of vertex buffer for the same primitive have different vertex counts!");
			}
		}
		else
		{
			previousVertexCount = vertexCount;
		}
	}

	OgreLog("There will be " + std::to_string(vertexCount) + " vertices with a stride of " + std::to_string(stride) + " bytes");

	Ogre_glTF_geometryBuffer<float> finalBuffer(vertexCount * strideInElements);
	size_t bytesWrittenInCurrentStride;
	for (size_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
	{
		bytesWrittenInCurrentStride = 0;
		for (const auto& part : parts)
		{
			memcpy(finalBuffer.dataAddress() + (bytesWrittenInCurrentStride + vertexIndex * stride),
				(part.buffer->dataAddress() + (vertexIndex * part.getPartStride())),
				part.getPartStride()
			);
			bytesWrittenInCurrentStride += part.getPartStride();
		}
	}

	//OgreLog("Final content of the buffer:");
	//finalBuffer._debugContentToLog();

	Ogre::VertexBufferPackedVec vec;
	auto vertexBuffer = Ogre_glTF_modelConverter::getVaoManager()->
		createVertexBuffer(vertexElements,
			vertexCount,
			Ogre::BT_IMMUTABLE,
			finalBuffer.data(),
			false);

	vec.push_back(vertexBuffer);
	return vec;
}

//TODO make this method thake the mesh id. Enumerate the meshes in the filel before blindlessly loading the first one
Ogre::MeshPtr Ogre_glTF_modelConverter::getOgreMesh()
{
	OgreLog("Default scene" + std::to_string(model.defaultScene));
	const auto mainMeshIndex = (model.defaultScene != 0 ? model.nodes[model.scenes[model.defaultScene].nodes.front()].mesh : 0);
	const auto& mesh = model.meshes[mainMeshIndex];
	OgreLog("Found mesh " + mesh.name + " in glTF file");

	auto OgreMesh = Ogre::MeshManager::getSingleton().getByName(mesh.name);
	if (OgreMesh)
	{
		OgreLog("Found mesh " + mesh.name + " in Ogre::MeshManager(v2)");
		return OgreMesh;
	}
	OgreLog("Loading mesh from glTF file");
	OgreLog("mesh has " + std::to_string(mesh.primitives.size()) + " primitives");
	OgreMesh = Ogre::MeshManager::getSingleton().createManual(mesh.name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	OgreLog("Created mesh on v2 MeshManager");
	for (const auto& primitive : mesh.primitives)
	{
		auto subMesh = OgreMesh->createSubMesh();
		OgreLog("Created one submesh");
		auto indexBuffer = extractIndexBuffer(primitive.indices);

		std::vector<Ogre_glTF_vertexBufferPart> parts;
		OgreLog("\tprimitive has : " + std::to_string(primitive.attributes.size()) + " atributes");
		for (const auto& atribute : primitive.attributes)
		{
			OgreLog("\t " + atribute.first);
			parts.push_back(std::move(extractVertexBuffer(atribute)));
		}

		auto vertexBuffers = constructVertexBuffer(parts);
		auto vao = getVaoManager()->createVertexArrayObject(vertexBuffers, indexBuffer, [&]
		{
			switch (primitive.mode)
			{
			case TINYGLTF_MODE_LINE:
				OgreLog("Line List");
				return Ogre::OT_LINE_LIST;
			case TINYGLTF_MODE_LINE_LOOP:
				OgreLog("Line Loop");
				return Ogre::OT_LINE_STRIP;
			case TINYGLTF_MODE_POINTS:
				OgreLog("Points");
				return Ogre::OT_POINT_LIST;
			case TINYGLTF_MODE_TRIANGLES:
				OgreLog("Triangle List");
				return Ogre::OT_TRIANGLE_LIST;
			case TINYGLTF_MODE_TRIANGLE_FAN:
				OgreLog("Trinagle Fan");
				return Ogre::OT_TRIANGLE_FAN;
			case TINYGLTF_MODE_TRIANGLE_STRIP:
				OgreLog("Triangle Strip");
				return Ogre::OT_TRIANGLE_STRIP;
			default:
				OgreLog("Unknown");
				throw std::runtime_error("Can't understand primitive mode!");
			};
		}());

		subMesh->mVao[Ogre::VpNormal].push_back(vao);
		subMesh->mVao[Ogre::VpShadow].push_back(vao);
	}

	//TODO use the min and max values of the mesh accessor to actually calculate AABB for this object
	OgreMesh->_setBounds(Ogre::Aabb(Ogre::Vector3::ZERO, Ogre::Vector3::UNIT_SCALE), false);
	OgreMesh->_setBoundingSphereRadius(1.732f);

	return OgreMesh;
}

void Ogre_glTF_modelConverter::debugDump() const
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
		<< model.lights.size() << " lights\n";

	OgreLog(ss.str());
}

bool Ogre_glTF_modelConverter::hasSkins() const
{
	if (model.skins.size() > 0) return true;
	return false;
}

Ogre::VaoManager* Ogre_glTF_modelConverter::getVaoManager()
{
	//Our class shouldn't be able to exist if Ogre hasn't been initalized with a valid render system. This call should allways succeed.
	return Ogre::Root::getSingletonPtr()->getRenderSystem()->getVaoManager();
}

Ogre::IndexBufferPacked* Ogre_glTF_modelConverter::extractIndexBuffer(int accessorID) const
{
	OgreLog("Extracting index buffer");
	const auto& accessor = model.accessors[accessorID];
	const auto& bufferView = model.bufferViews[accessor.bufferView];
	auto& buffer = model.buffers[bufferView.buffer];
	const auto byteStride = accessor.ByteStride(bufferView);
	const auto indexCount = accessor.count;
	Ogre::IndexBufferPacked::IndexType type;

	if (byteStride < 0)
		throw std::runtime_error("Can't get valid bytestride from accessor and bufferview. Loading data not possible");

	auto convertTo16Bit{ false };
	switch (accessor.componentType)
	{
	default:
		throw std::runtime_error("Unrecognized index data format");
	case TINYGLTF_COMPONENT_TYPE_BYTE:
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
		convertTo16Bit = true;
	case TINYGLTF_COMPONENT_TYPE_SHORT:
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
	{
		type = Ogre::IndexBufferPacked::IT_16BIT;
		auto geometryBuffer = Ogre_glTF_geometryBuffer<Ogre::uint16>(indexCount);
		if (convertTo16Bit) loadIndexBuffer(geometryBuffer.data(), buffer.data.data(), indexCount, bufferView.byteOffset + accessor.byteOffset, byteStride);
		else loadIndexBuffer(geometryBuffer.data(), reinterpret_cast<Ogre::uint16*>(buffer.data.data()), indexCount, bufferView.byteOffset + accessor.byteOffset, byteStride);
		return getVaoManager()->createIndexBuffer(type, indexCount, Ogre::BT_IMMUTABLE, geometryBuffer.dataAddress(), false);
	}
	case TINYGLTF_COMPONENT_TYPE_INT:;
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
	{
		type = Ogre::IndexBufferPacked::IT_32BIT;
		auto geometryBuffer = Ogre_glTF_geometryBuffer<Ogre::uint32>(indexCount);
		loadIndexBuffer(geometryBuffer.data(), reinterpret_cast<Ogre::uint32*>(buffer.data.data()), indexCount, bufferView.byteOffset + accessor.byteOffset, byteStride);
		return getVaoManager()->createIndexBuffer(type, indexCount, Ogre::BT_IMMUTABLE, geometryBuffer.dataAddress(), false);
	}
	}
}

size_t Ogre_glTF_modelConverter::getVertexBufferElementsPerVertexCount(int type)
{
	switch (type)
	{
	case TINYGLTF_TYPE_VEC2: return 2;
	case TINYGLTF_TYPE_VEC3: return 3;
	case TINYGLTF_TYPE_VEC4: return 4;
	default: return 0;
	}
}

Ogre::VertexElementSemantic Ogre_glTF_modelConverter::getVertexElementScemantic(const std::string& type)
{
	if (type == "POSITION") return Ogre::VES_POSITION;
	if (type == "NORMAL") return Ogre::VES_NORMAL;
	if (type == "TANGENT") return Ogre::VES_TANGENT;
	if (type == "TEXCOORD_0") return Ogre::VES_TEXTURE_COORDINATES;
	if (type == "TEXCOORD_1") return Ogre::VES_TEXTURE_COORDINATES;
	if (type == "COLOR_0") return Ogre::VES_DIFFUSE;
	if (type == "JOINTS_0") return Ogre::VES_BLEND_INDICES;
	if (type == "WEIGHTS_0") return Ogre::VES_BLEND_WEIGHTS;
	return Ogre::VES_COUNT; //Returning this means returning "invalid" here
}

Ogre_glTF_vertexBufferPart Ogre_glTF_modelConverter::extractVertexBuffer(const std::pair<std::string, int>& attribute) const
{
	const auto elementScemantic = getVertexElementScemantic(attribute.first);
	const auto& accessor = model.accessors[attribute.second];
	const auto& bufferView = model.bufferViews[accessor.bufferView];
	const auto& buffer = model.buffers[bufferView.buffer];
	const auto vertexBufferByteLen = bufferView.byteLength;
	const auto numberOfElementPerVertex = getVertexBufferElementsPerVertexCount(accessor.type);
	const auto elementOffsetInBuffer = bufferView.byteOffset + accessor.byteOffset;
	size_t bufferLenghtInBufferBasicType;

	std::unique_ptr<Ogre_glTF_geometryBuffer_base> geometryBuffer{ nullptr };
	Ogre::VertexElementType elementType{};

	switch (accessor.componentType)
	{
	case TINYGLTF_COMPONENT_TYPE_DOUBLE:
		throw std::runtime_error("Double pressision not implemented!");
	case TINYGLTF_COMPONENT_TYPE_FLOAT:
		bufferLenghtInBufferBasicType = (vertexBufferByteLen / sizeof(float));
		geometryBuffer = std::make_unique<Ogre_glTF_geometryBuffer<float>>(bufferLenghtInBufferBasicType);
		if (numberOfElementPerVertex == 2) elementType = Ogre::VET_FLOAT2;
		if (numberOfElementPerVertex == 3) elementType = Ogre::VET_FLOAT3;
		if (numberOfElementPerVertex == 4) elementType = Ogre::VET_FLOAT4;
		break;
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
		bufferLenghtInBufferBasicType = (vertexBufferByteLen / sizeof(unsigned short));
		geometryBuffer = std::make_unique<Ogre_glTF_geometryBuffer<unsigned short>>(bufferLenghtInBufferBasicType);
		if (numberOfElementPerVertex == 2) elementType = Ogre::VET_USHORT2;
		if (numberOfElementPerVertex == 4) elementType = Ogre::VET_USHORT4;
		break;
	default:
		throw std::runtime_error("Unrecognized vertex buffer coponent type");
	}


	if (bufferView.byteStride == 0) OgreLog("Vertex buffer is 'tightly packed'");
	const auto byteStride = accessor.ByteStride(bufferView);
	if (byteStride < 0) throw std::runtime_error("Can't get valid bytestride from accessor and bufferview. Loading data not possible");
	const auto vertexCount = accessor.count;

	const auto vertexElementLenghtInBytes = numberOfElementPerVertex * geometryBuffer->elementSize();
	OgreLog("A vertex element on this buffer is " + std::to_string(vertexElementLenghtInBytes) + " bytes long");
	for (size_t vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
	{
		const size_t destOffset = vertexIndex * vertexElementLenghtInBytes;
		const size_t sourceOffset = elementOffsetInBuffer + vertexIndex * byteStride;

		memcpy((geometryBuffer->dataAddress() + destOffset),
			(buffer.data.data() + sourceOffset),
			vertexElementLenghtInBytes);
	}

	//geometryBuffer->_debugContentToLog();

	return {
		std::move(geometryBuffer),
		elementType,
		elementScemantic,
		vertexCount,
		numberOfElementPerVertex
	};
}