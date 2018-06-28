#include "Ogre_glTF_skeletonImporter.hpp"
#include "Ogre_glTF_common.hpp"
#include "Ogre_glTF_internal_utils.hpp"
#include <OgreOldSkeletonManager.h>
#include <OgreSkeleton.h>
#include <OgreOldBone.h>
#include <OgreLogManager.h>


int Ogre_glTF_skeletonImporter::skeletonID = 0;

void Ogre_glTF_skeletonImporter::addChidren(const std::string& skinName, const std::vector<int>& childs, Ogre::v1::OldBone* parent)
{
	OgreLog("Bone " + std::to_string(parent->getHandle()) + " has " + std::to_string(childs.size()) + " children");
	for(auto child : childs)
	{
		const auto& node = model.nodes[child];
		OgreLog("Node name is " + node.name + "!");
		auto toFloat = [](double n) { return static_cast<float>(n); };

		std::array<float, 3> translation{};
		std::array<float, 4> rotation{};

		internal_utils::container_double_to_float(node.translation, translation);
		internal_utils::container_double_to_float(node.rotation, rotation);


		auto bone = skeleton->getBone(child - offset);
		if(!bone)
		{
			throw std::runtime_error("could not get bone " + std::to_string(child - offset));
		}

		parent->addChild(bone);

		bone->setPosition(Ogre::Vector3{ translation.data() });
		bone->setOrientation(Ogre::Quaternion{ rotation.data() });

		//auto bone = parent->createChild(child - offset, Ogre::Vector3{ translation.data() }, Ogre::Quaternion{ rotation.data() });
		Ogre::LogManager::getSingleton().logMessage("Bone pointer value : " + std::to_string(std::size_t(bone)));

		addChidren(skinName + std::to_string(child), model.nodes[child].children, bone);
	}
}

void Ogre_glTF_skeletonImporter::loadBoneHierarchy(const tinygltf::Skin& skin, Ogre::v1::OldBone* rootBone, const std::string& name)
{
	const auto& node = model.nodes[skin.joints[0]];
	const auto& skeleton = model.nodes[skin.skeleton];

	std::array<float, 3> translation{};
	std::array<float, 4> rotation{};

	internal_utils::container_double_to_float(skeleton.translation, translation);
	internal_utils::container_double_to_float(skeleton.rotation, rotation);

	rootBone->setPosition(Ogre::Vector3{ translation.data() });
	rootBone->setOrientation(Ogre::Quaternion{ rotation.data() });

	addChidren(name, node.children, rootBone);
}

Ogre_glTF_skeletonImporter::Ogre_glTF_skeletonImporter(tinygltf::Model& input) :
 model{ input }
{
}

Ogre::v1::SkeletonPtr Ogre_glTF_skeletonImporter::getSkeleton()
{
	const auto& skins		 = model.skins;

	assert(skins.size() > 0);
	const auto skin = skins[0];

	std::string skeletonName;

	if(!skin.name.empty())
		skeletonName = skin.name;
	else
		skeletonName = "unnamedSkeleton" + std::to_string(skeletonID++); 

	OgreLog("First skin name is " + skeletonName);

	skeleton = Ogre::v1::OldSkeletonManager::getSingleton().getByName(skeletonName);
	if(skeleton)
	{
		OgreLog("Found in the skeleton manager");
		return skeleton;
	}

	skeleton = Ogre::v1::OldSkeletonManager::getSingleton().create(skeletonName,
																   Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
																   true);

	if(!skeleton)
	{
		throw std::runtime_error("Coudn't create skeletion for skin" + skeletonName);
	}

	const auto firstJoint = skin.joints[0];
	offset = firstJoint;
	//auto rootBone = skeleton->createBone(skeletonName + std::to_string(firstJoint), firstJoint - offset);
	OgreLog("skin.skeleton (root joint) = " + std::to_string(skin.skeleton));
	for(auto joint : skin.joints)
	{
		const auto name = model.nodes[joint].name;
		OgreLog("joint " + std::to_string(joint) + " name: " + name);
		skeleton->createBone((!name.empty()) ? name : skeletonName + std::to_string(joint - offset), joint - offset);
	}

	auto rootBone = skeleton->getBone(0);

	//set offset

	loadBoneHierarchy(skin, rootBone, skeletonName);

	return skeleton;
}
