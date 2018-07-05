#include <Ogre.h>
//To create workspace definitions and workspaces
#include <Compositor/OgreCompositorManager2.h>
//To use the hlms
#include <Hlms/Pbs/OgreHlmsPbs.h>
#include <Hlms/Unlit/OgreHlmsUnlit.h>
#include <OgreHlms.h>
//To load Hlms
#include <OgreArchive.h>
//To use objects
#include <OgreItem.h>
//To play animations
#include <Animation/OgreSkeletonAnimation.h>
//To use smart pointers
#include <memory>

//The library we are trying out in this program
#include <Ogre_glTF.hpp>

#ifdef _DEBUG
const char GL_RENDER_PLUGIN[] = "RenderSystem_GL3Plus_d";
#else
const char GL_RENDER_PLUGIN[] = "RenderSystem_GL3Plus";
#endif

#ifdef _WIN32
#ifdef _DEBUG
const char D3D11_RENDER_PLUGIN[] = "RenderSystem_Direct3D11_d";
#else
const char D3D11_RENDER_PLUGIN[] = "RenderSystem_Direct3D11";
#endif
#endif

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#define main() WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
#define main() main(int argc, char* argv[])
#endif

void declareHlmsLibrary(Ogre::String dataFolder)
{
	//Make sure the string we got is a valid path
	if(dataFolder.empty())
		dataFolder = "./";
	else if(dataFolder[dataFolder.size() - 1] != '/')
		dataFolder += '/';

	//For retrieval of the paths to the different folders needed
	Ogre::String dataFolderPath;
	Ogre::StringVector libraryFoldersPaths;

#ifdef USE_UNLIT //We are loading materials based on physics
	//Get the path to all the subdirectories used by HlmsUnlit
	Ogre::HlmsUnlit::getDefaultPaths(dataFolderPath, libraryFoldersPaths);

	//Create the Ogre::Archive objects needed
	Ogre::Archive* archiveUnlit = Ogre::ArchiveManager::getSingletonPtr()->load(dataFolder + dataFolderPath, "FileSystem", true);
	Ogre::ArchiveVec archiveUnlitLibraryFolders;
	for(const auto& libraryFolderPath : libraryFoldersPaths)
	{
		Ogre::Archive* archiveLibrary = Ogre::ArchiveManager::getSingletonPtr()->load(dataFolder + libraryFolderPath, "FileSystem", true);
		archiveUnlitLibraryFolders.push_back(archiveLibrary);
	}

	//Create and register the unlit Hlms
	Ogre::HlmsUnlit* hlmsUnlit = OGRE_NEW Ogre::HlmsUnlit(archiveUnlit, &archiveUnlitLibraryFolders);
	Ogre::Root::getSingleton().getHlmsManager()->registerHlms(hlmsUnlit);
	hlmsUnlit->setDebugOutputPath(false, false);
#endif

	//Do the same for HlmsPbs:
	Ogre::HlmsPbs::getDefaultPaths(dataFolderPath, libraryFoldersPaths);
	Ogre::Archive* archivePbs = Ogre::ArchiveManager::getSingletonPtr()->load(dataFolder + dataFolderPath, "FileSystem", true);

	//Get the library archive(s)
	Ogre::ArchiveVec archivePbsLibraryFolders;
	for(const auto& libraryFolderPath : libraryFoldersPaths) {
		Ogre::Archive* archiveLibrary = Ogre::ArchiveManager::getSingletonPtr()->load(dataFolder + libraryFolderPath, "FileSystem", true);
		archivePbsLibraryFolders.push_back(archiveLibrary);
	}

	//Create and register
	Ogre::HlmsPbs* hlmsPbs = OGRE_NEW Ogre::HlmsPbs(archivePbs, &archivePbsLibraryFolders);
	Ogre::Root::getSingleton().getHlmsManager()->registerHlms(hlmsPbs);
	hlmsPbs->setDebugOutputPath(false, false);
}

int main()
{
	//Init Ogre
	auto root = std::make_unique<Ogre::Root>();
	Ogre::LogManager::getSingleton().setLogDetail(Ogre::LoggingLevel::LL_BOREME);

	root->loadPlugin(GL_RENDER_PLUGIN);
#ifdef _WIN32
	root->loadPlugin(D3D11_RENDER_PLUGIN);
#endif
	root->showConfigDialog();
	root->getRenderSystem()->setConfigOption("FSAA", "16");
	root->getRenderSystem()->setConfigOption("sRGB Gamma Conversion", "Yes");
	root->initialise(false);

	//Create a window and a scene
	Ogre::NameValuePairList params;
	params["FSAA"] = "16";
	auto window	= root->createRenderWindow("glTF test!", 800, 600, false, &params);
	auto smgr	  = root->createSceneManager(Ogre::ST_GENERIC, 2, Ogre::INSTANCING_CULLING_THREADED);
	auto camera	= smgr->createCamera("cam");

	//Setup rendering pipeline
	auto compositor			   = root->getCompositorManager2();
	const char workspaceName[] = "workspace0";
	compositor->createBasicWorkspaceDef(workspaceName, Ogre::ColourValue{ 0.2f, 0.3f, 0.4f });
	auto workspace = compositor->addWorkspace(smgr, window, camera, workspaceName, true);

	declareHlmsLibrary("./");

	//Ogre::ResourceGroupManager::getSingleton().addResourceLocation("./media", "FileSystem");
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups(true);

	Ogre::Item* ObjectItem		= nullptr;
	Ogre::SceneNode* ObjectNode = nullptr;

	//Ogre::Item* OtherItem;
	//Initialize the library
	auto gltf = std::make_unique<Ogre_glTF>();
	try
	{
		//auto adapter = gltf->loadFile("from_gltf_export_skinned_cylinder.glb");
		auto adapter = gltf->loadFile("RiggedSimple.glb");
		//auto adapter = gltf->loadFile("./damagedHelmet/damagedHelmet.gltf");
		//auto adapter = gltf->loadFile("./Corset.glb");
		ObjectItem
			= adapter.getItem(smgr);
		//OtherItem = adapter.getItem(smgr);
	}
	catch(std::exception& e)
	{
		Ogre::LogManager::getSingleton().logMessage(e.what());
		return -1;
	}

	ObjectNode = smgr->getRootSceneNode()->createChildSceneNode();
	ObjectNode->attachObject(ObjectItem);
	//auto OtherNode = smgr->getRootSceneNode()->createChildSceneNode();
	//OtherNode->attachObject(OtherItem);
	//OtherNode->setPosition(-2, 0, 0);
	camera->setNearClipDistance(0.001);
	camera->setFarClipDistance(100);
	//camera->setPosition(Ogre::Vector3::UNIT_SCALE * 0.0625);
	//camera->lookAt({ 0, 0.03125, 0 });
	camera->setPosition(Ogre::Vector3::UNIT_SCALE * 5);
	camera->lookAt({ 0, 0, 0 });
	camera->setAutoAspectRatio(true);

	auto light = smgr->createLight();
	smgr->getRootSceneNode()->createChildSceneNode()->attachObject(light);
	light->setType(Ogre::Light::LT_DIRECTIONAL);
	light->setDirection({ -1, -1, -0.5 });
	light->setPowerScale(5);

	auto skeleton = ObjectItem->getSkeletonInstance();

	//Ogre::Bone* bone = nullptr;
	//if(skeleton)
	//{
	//	Ogre::LogManager::getSingleton().logMessage("skeleton instance? :O");
	//	if(skeleton->getBone(0))
	//		bone = skeleton->getBone(0)->getChild(0);
	//}
	//if(bone)
	//	skeleton->setManualBone(bone, true);

	//Ogre::LogManager::getSingleton().logMessage("Bone pointer value : " + std::to_string(std::size_t(bone)));

	Ogre::SkeletonAnimation* anim = nullptr;
	if(skeleton)
	{
		auto& animationList			  = skeleton->getAnimations();
		if(!animationList.empty())
		{
			auto name = animationList[0].getName();
			anim	  = skeleton->getAnimation(name);
		}

		if(anim)
		{
			anim->setEnabled(true);
			anim->setLoop(true);
		}
	}

	auto last = root->getTimer()->getMilliseconds();
	auto now  = last;
	while(!window->isClosed()) {
		if(skeleton)
		for(auto i = 0; i < skeleton->getNumBones(); ++i)
		{
			auto a_bone = skeleton->getBone(i);
			std::stringstream ss;
			ss << "bone " << i << " position " << a_bone->getPosition() << " orientaiton " << a_bone->getOrientation();
			Ogre::LogManager::getSingleton().logMessage(ss.str());
		}

		//if(bone)
		//{
		//	bone->setOrientation(bone->getParent()->getOrientation() * Ogre::Quaternion(Ogre::Degree( 45.0f * float(sin((float)root->getTimer()->getMilliseconds() / 1000.0f))), Ogre::Vector3::UNIT_Z));
		//	bone->setPosition({ 0, 2.0f * sin((float)root->getTimer()->getMilliseconds() / 1000.f), 0 });
		//}

		//ObjectNode->setOrientation(Ogre::Quaternion(Ogre::Degree(float(root->getTimer()->getMilliseconds()) / 10.0f), Ogre::Vector3::NEGATIVE_UNIT_Y));
		//OtherNode->setOrientation(Ogre::Quaternion(Ogre::Degree(float(root->getTimer()->getMilliseconds()) / 10.0f), Ogre::Vector3::UNIT_Y));

		now = root->getTimer()->getMilliseconds();
		if(anim)
			anim->addTime(float(now - last) / 1000.0f);
		last = now;

		root->renderOneFrame();
		Ogre::WindowEventUtilities::messagePump();
	}

	return 0;
}
