#include "Ogre_glTF_skeletonImporter.hpp"
#include <OgreOldSkeletonManager.h>
#include <OgreSkeleton.h>
#include "Ogre_glTF_common.hpp"

Ogre_glTF_skeletonImporter::Ogre_glTF_skeletonImporter(tinygltf::Model& input) :
model{input}
{
}

Ogre::v1::SkeletonPtr Ogre_glTF_skeletonImporter::getSkeleton()
{
	const auto mainMeshIndex = (model.defaultScene != 0 ? model.nodes[model.scenes[model.defaultScene].nodes.front()].mesh : 0);
	const auto& mesh = model.meshes[mainMeshIndex];
	const auto& skins = model.skins;

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
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, true);

	if(!skeleton)
	{
		OgreLog("skeleton pointer is still not valid?");
	}


	return skeleton;
}
