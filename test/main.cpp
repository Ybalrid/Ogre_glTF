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

//To use smart pointers
#include <memory>

//The library we are trying out in this program
#include <Ogre_glTF.hpp>

#ifdef _DEBUG
const char RENDER_PLUGIN[] = "RenderSystem_GL3Plus_d";
#else
const char RENDER_PLUGIN[] = "RenderSystem_GL3Plus";
#endif

void declareHlmsLibrary(Ogre::String dataFolder)
{
	//Make sure the string we got is a valid path
	if (dataFolder.empty()) dataFolder = "./";
	else if (dataFolder[dataFolder.size() - 1] != '/') dataFolder += '/';

	//For retrieval of the paths to the different folders needed
	Ogre::String dataFolderPath;
	Ogre::StringVector libraryFoldersPaths;

#ifdef USE_UNLIT //We are loading materials based on physics
	//Get the path to all the subdirectories used by HlmsUnlit
	Ogre::HlmsUnlit::getDefaultPaths(dataFolderPath, libraryFoldersPaths);

	//Create the Ogre::Archive objects needed
	Ogre::Archive* archiveUnlit = Ogre::ArchiveManager::getSingletonPtr()->load(dataFolder + dataFolderPath, "FileSystem", true);
	Ogre::ArchiveVec archiveUnlitLibraryFolders;
	for (const auto& libraryFolderPath : libraryFoldersPaths)
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
	for (const auto& libraryFolderPath : libraryFoldersPaths)
	{
		Ogre::Archive* archiveLibrary = Ogre::ArchiveManager::getSingletonPtr()->load(dataFolder + libraryFolderPath, "FileSystem", true);
		archivePbsLibraryFolders.push_back(archiveLibrary);
	}

	//Create and register
	Ogre::HlmsPbs* hlmsPbs = OGRE_NEW Ogre::HlmsPbs(archivePbs, &archivePbsLibraryFolders);
	Ogre::Root::getSingleton().getHlmsManager()->registerHlms(hlmsPbs);
	hlmsPbs->setDebugOutputPath(false, false);
}

#ifdef _WIN32
#include <windows.h>
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
int main(int argc, char* argv[])
#endif
{
	//Init Ogre
	auto root = std::make_unique<Ogre::Root>();
	Ogre::LogManager::getSingleton().setLogDetail(Ogre::LoggingLevel::LL_BOREME);

	root->loadPlugin(RENDER_PLUGIN);
	root->setRenderSystem(root->getAvailableRenderers().front());
	root->initialise(false);

	//Create a window and a scene
	auto window = root->createRenderWindow("glTF test!", 800, 600, false, nullptr);
	auto smgr = root->createSceneManager(Ogre::ST_GENERIC, 2, Ogre::INSTANCING_CULLING_THREADED);
	auto camera = smgr->createCamera("cam");

	//Setup rendering pipeline
	auto compositor = root->getCompositorManager2();
	const char workspaceName[] = "workspace0";
	compositor->createBasicWorkspaceDef(workspaceName, Ogre::ColourValue{ 0.2f, 0.3f, 0.4f });
	auto workspace = compositor->addWorkspace(smgr, window, camera, workspaceName, true);

	declareHlmsLibrary("./");

	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups(true);

	Ogre::Item* CorsetItem = nullptr;
	Ogre::SceneNode* CorsetNode = nullptr;

	//Initialize the library
	auto gltf = std::make_unique<Ogre_glTF>();
	try
	{
		auto adapter = gltf->loadFile("./Corset.glb");
		CorsetItem = adapter.getItem(smgr);
	}
	catch (std::exception& e)
	{
		Ogre::LogManager::getSingleton().logMessage(e.what());
	}

	CorsetNode = smgr->getRootSceneNode()->createChildSceneNode();
	CorsetNode->attachObject(CorsetItem);
	camera->setNearClipDistance(0.1);
	camera->setFarClipDistance(100);
	camera->setPosition({ 5, 5, 5 });
	camera->lookAt({ 0, 0, 0 });

	auto light = smgr->createLight();
	smgr->getRootSceneNode()->createChildSceneNode()->attachObject(light);
	light->setType(Ogre::Light::LT_DIRECTIONAL);
	light->setDirection({ -1, -1, -0.5 });

	while (!window->isClosed())
	{
		Ogre::WindowEventUtilities::messagePump();
		root->renderOneFrame();
	}

	//TODO FIX Looks like the program is haning somewher at this point. This doesn't make sense

	return 0;
}