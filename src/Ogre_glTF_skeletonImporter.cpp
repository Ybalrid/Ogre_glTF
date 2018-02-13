#include "Ogre_glTF_skeletonImporter.hpp"
#include "Ogre_glTF_common.hpp"
#include "Ogre_glTF_internal_utils.hpp"
#include <OgreOldSkeletonManager.h>
#include <OgreSkeleton.h>
#include <OgreOldBone.h>

void Ogre_glTF_skeletonImporter::addChidren(const std::string& skinName, const std::vector<int>& childs, Ogre::v1::OldBone* parent)
{
	OgreLog("Bone " + std::to_string(parent->getHandle()) + " has " + std::to_string(childs.size()) + " children");
	for(auto child : childs)
	{
		auto bone		 = parent->createChild(child);
		const auto& node = model.nodes[child];

		auto toFloat = [](double n) { return static_cast<float>(n); };

		std::array<float, 3> translation;
		std::array<float, 4> rotation;

		internal_utils::container_to_float(node.translation, translation);
		internal_utils::container_to_float(node.rotation, rotation);

		bone->setPosition(Ogre::Vector3{ translation.data() });
		bone->setOrientation(Ogre::Quaternion{ rotation.data() });

		addChidren(skinName, model.nodes[child].children, bone);
	}
}

void Ogre_glTF_skeletonImporter::loadBoneHierarchy(const tinygltf::Skin& skin, Ogre::v1::OldBone* rootBone)
{
	const auto& node = model.nodes[skin.skeleton];

	auto toFloat = [](double n) { return static_cast<float>(n); };

	std::array<float, 3> translation;
	std::array<float, 4> rotation;

	internal_utils::container_to_float(node.translation, translation);
	internal_utils::container_to_float(node.rotation, rotation);

	rootBone->setPosition(Ogre::Vector3{ translation.data() });
	rootBone->setOrientation(Ogre::Quaternion{ rotation.data() });

	addChidren(skin.name, node.children, rootBone);
}

Ogre_glTF_skeletonImporter::Ogre_glTF_skeletonImporter(tinygltf::Model& input) :
 model{ input }
{
}

Ogre::v1::SkeletonPtr Ogre_glTF_skeletonImporter::getSkeleton()
{
	const auto mainMeshIndex = (model.defaultScene != 0 ? model.nodes[model.scenes[model.defaultScene].nodes.front()].mesh : 0);
	const auto& mesh		 = model.meshes[mainMeshIndex];
	const auto& skins		 = model.skins;

	assert(skins.size() > 0);
	const auto skin = skins[0];

	OgreLog("First skin name is " + skin.name);

	auto skeleton = Ogre::v1::OldSkeletonManager::getSingleton().getByName(skin.name);
	if(skeleton)
	{
		OgreLog("Found in the skeleton manager");
		return skeleton;
	}

	skeleton = Ogre::v1::OldSkeletonManager::getSingleton().create(skin.name,
																   Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
																   true);

	if(!skeleton)
	{
		OgreLog("skeleton pointer is still not valid?");
	}

	OgreLog("skin.skeleton (root joint) = " + std::to_string(skin.skeleton));
	for(auto joint : skin.joints)
		OgreLog("joint " + std::to_string(joint));

	auto rootBone = skeleton->createBone(skin.name + std::to_string(skin.skeleton), skin.skeleton);
	loadBoneHierarchy(skin, rootBone);

	return skeleton;
}
