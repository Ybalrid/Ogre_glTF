#include <Ogre.h>
#include <OgreHlmsPbs.h>
#include <Compositor/OgreCompositorManager2.h>
#include <OgreItem.h>

#include "Ogre_glTF_OgrePlugin.hpp"

#ifdef _DEBUG
const char GL_RENDER_PLUGIN[] = "./RenderSystem_GL3Plus_d";
#else
const char GL_RENDER_PLUGIN[] = "./RenderSystem_GL3Plus";
#endif

#ifdef _WIN32
#ifdef _DEBUG
const char D3D11_RENDER_PLUGIN[] = "./RenderSystem_Direct3D11_d";
const char Ogre_glTF_PluginPath[] = "./Debug/Ogre_glTF_d";
#else
const char D3D11_RENDER_PLUGIN[] = "./RenderSystem_Direct3D11";
const char Ogre_glTF_PluginPath[] = "./Release/Ogre_glTF";
#endif
#endif

//Redefine the entry point of the program to use int main() on Windows
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#define main() WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
#define main() main(int argc, char* argv[])
#endif

using namespace Ogre;
using namespace Ogre_glTF;

void declareHlmsLibrary(String dataFolder)
{
	//Make sure the string we got is a valid path
	if(dataFolder.empty())
		dataFolder = "./";
	else if(dataFolder[dataFolder.size() - 1] != '/')
		dataFolder += '/';

	//For retrieval of the paths to the different folders needed
	String dataFolderPath;
	StringVector libraryFoldersPaths;

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
	HlmsPbs::getDefaultPaths(dataFolderPath, libraryFoldersPaths);
	Archive* archivePbs = ArchiveManager::getSingletonPtr()->load(dataFolder + dataFolderPath, "FileSystem", true);

	//Get the library archive(s)
	ArchiveVec archivePbsLibraryFolders;
	for(const auto& libraryFolderPath : libraryFoldersPaths)
	{
		Archive* archiveLibrary = ArchiveManager::getSingletonPtr()->load(dataFolder + libraryFolderPath, "FileSystem", true);
		archivePbsLibraryFolders.push_back(archiveLibrary);
	}

	//Create and register
	HlmsPbs* hlmsPbs = OGRE_NEW HlmsPbs(archivePbs, &archivePbsLibraryFolders);
	Root::getSingleton().getHlmsManager()->registerHlms(hlmsPbs);
	hlmsPbs->setDebugOutputPath(false, false);
}

CompositorWorkspace* setupCompositor(Root* root, RenderWindow* const window, SceneManager* smgr, Camera* camera)
{
	//Setup rendering pipeline
	auto compositor			   = root->getCompositorManager2();
	const char workspaceName[] = "workspace0";
	compositor->createBasicWorkspaceDef(workspaceName, ColourValue { 0.2f, 0.3f, 0.4f });
	auto workspace = compositor->addWorkspace(smgr, window, camera, workspaceName, true);

	return workspace;
}

int main()
{
	//Create Ogre's root, and load every plugins you normally do (could use plugin.cfg for same effect)
	auto root = std::make_unique<Root>();
	LogManager::getSingleton().setLogDetail(LL_BOREME);
	root->loadPlugin(GL_RENDER_PLUGIN);
#ifdef _WIN32
	root->loadPlugin(D3D11_RENDER_PLUGIN);
	//Append the Ogre_glTF plugin to that list
	root->loadPlugin(Ogre_glTF_PluginPath);
#else
    root->loadPlugin("./libOgre_glTF.so");
#endif


	//Startup Ogre as you would generally do it
	root->showConfigDialog();
	root->getRenderSystem()->setConfigOption("FSAA", "16");
	root->getRenderSystem()->setConfigOption("sRGB Gamma Conversion", "Yes");
	root->initialise(false);

	//Create a window and a scene
	NameValuePairList params;
	params["FSAA"]	= "16";
	params["gamma"]   = "true";
	const auto window = root->createRenderWindow("glTF test!", 800, 600, false, &params);
	auto smgr		  = root->createSceneManager(ST_GENERIC, 2, INSTANCING_CULLING_THREADED);
	smgr->setForward3D(true, 4, 4, 5, 96, 3, 200);
	auto camera = smgr->createCamera("cam");
	camera->setNearClipDistance(0.001f);
	camera->setFarClipDistance(100);
	camera->setAutoAspectRatio(true);
	camera->setPosition(2, 2, 2);
	camera->lookAt(0, 1, 0);

	//Load workspace and hlms
	auto workspace = setupCompositor(root.get(), window, smgr, camera);
	declareHlmsLibrary("./");

	//Init resources
	ResourceGroupManager::getSingleton().addResourceLocation("gltfFiles.zip", "Zip");
	ResourceGroupManager::getSingleton().initialiseAllResourceGroups(true);

	//Get access to the gltf loader, and load a GLB file in the resources to an item
	auto glTFLoader = gltfPluginAccessor::findPlugin()->getLoader();
	auto item		= glTFLoader->getItemFromResource("CesiumMan.glb", smgr);
	auto itemNode   = smgr->getRootSceneNode()->createChildSceneNode();
	itemNode->attachObject(item);
	itemNode->setOrientation(Quaternion(Radian(Math::HALF_PI), Vector3::NEGATIVE_UNIT_X));

	//Add light
	auto light = smgr->createLight();
	light->setType(Light::LightTypes::LT_POINT);
	light->setPowerScale(5);
	auto lightNode = smgr->getRootSceneNode()->createChildSceneNode();
	lightNode->attachObject(light);
	lightNode->setPosition(1, 1, 1);

	//Setup animation and how to update it with time
	const auto animationName = (item->getSkeletonInstance()->getAnimations().front().getName());
	auto animation			 = item->getSkeletonInstance()->getAnimation(animationName);
	animation->setEnabled(true);
	animation->setLoop(true);
	auto timer	= root->getTimer();
	auto now	  = timer->getMilliseconds();
	auto last	 = now;
	auto duration = now - last;

	//Update
	while(!window->isClosed())
	{
		//Add time to animation
		now		 = timer->getMilliseconds();
		duration = (now - last);
		animation->addTime(Real(duration) / 1000.0f);
		last = now;

		//Render
		root->renderOneFrame();
		WindowEventUtilities::messagePump();
	}
}
