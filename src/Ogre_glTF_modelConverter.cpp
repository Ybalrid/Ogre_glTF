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

Ogre::IndexBufferPacked* Ogre_glTF_modelConverter::extractIndexBuffer(int accessorID)
{
	auto vaoManager{ getVaoManager() };

	auto& accessor = model.accessors[accessorID];
	auto& bufferView = model.bufferViews[accessor.bufferView];

	size_t elementSize;

	//TODO implement loading of index buffer
	switch (accessor.componentType)
	{
	case TINYGLTF_COMPONENT_TYPE_BYTE:
		break;
	}

	return nullptr;
}

Ogre::VertexBufferPacked* Ogre_glTF_modelConverter::extractVertexBuffer()
{
	auto vaoManager{ getVaoManager() };

	return nullptr;
}