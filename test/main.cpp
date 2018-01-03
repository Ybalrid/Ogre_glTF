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
//To load Ogre v1 meshes
#include <OgreMeshManager2.h>
#include <OgreMesh2.h>
#include <OgreMesh.h>
#include <OgreMeshManager.h>

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

decltype(auto) loadV1mesh(Ogre::String meshName)
{
	return Ogre::v1::MeshManager::getSingleton().load(meshName, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, Ogre::v1::HardwareBuffer::HBU_STATIC, Ogre::v1::HardwareBuffer::HBU_STATIC);
}

decltype(auto) asV2mesh(Ogre::String meshName, Ogre::String ResourceGroup = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::String sufix = " V2",
	bool halfPos = true, bool halfTextCoords = true, bool qTangents = true)
{
	//Get the V1 mesh
	auto v1mesh = loadV1mesh(meshName);

	//Convert it as a V2 mesh
	auto mesh = Ogre::MeshManager::getSingletonPtr()->createManual(meshName + sufix, ResourceGroup);
	mesh->importV1(v1mesh.get(), halfPos, halfTextCoords, qTangents);

	//Unload the useless V1 mesh
	v1mesh->unload();
	v1mesh.setNull();

	//Return the shared pointer to the new mesh
	return mesh;
}

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

	root->loadPlugin(GL_RENDER_PLUGIN);
#ifdef _WIN32
	root->loadPlugin(D3D11_RENDER_PLUGIN);
#endif
	//root->setRenderSystem(root->getAvailableRenderers().front());
	root->showConfigDialog();
	root->getRenderSystem()->setConfigOption("FSAA", "16");
	root->getRenderSystem()->setConfigOption("sRGB Gamma Conversion", "Yes");
	root->initialise(false);

	//Create a window and a scene
	Ogre::NameValuePairList params;
	params["FSAA"] = "16";
	auto window = root->createRenderWindow("glTF test!", 800, 600, false, &params);
	auto smgr = root->createSceneManager(Ogre::ST_GENERIC, 2, Ogre::INSTANCING_CULLING_THREADED);
	auto camera = smgr->createCamera("cam");

	//Setup rendering pipeline
	auto compositor = root->getCompositorManager2();
	const char workspaceName[] = "workspace0";
	compositor->createBasicWorkspaceDef(workspaceName, Ogre::ColourValue{ 0.2f, 0.3f, 0.4f });
	auto workspace = compositor->addWorkspace(smgr, window, camera, workspaceName, true);

	declareHlmsLibrary("./");

	//Ogre::ResourceGroupManager::getSingleton().addResourceLocation("./media", "FileSystem");
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups(true);

	Ogre::Item* ObjectItem = nullptr;
	Ogre::SceneNode* ObjectNode = nullptr;

	//Initialize the library
	auto gltf = std::make_unique<Ogre_glTF>();
	try
	{
		auto adapter = gltf->loadFile("./damagedHelmet/damagedHelmet.gltf");
		//auto adapter = gltf->loadFile("./Corset.glb");
		ObjectItem = adapter.getItem(smgr);
	}
	catch (std::exception& e)
	{
		Ogre::LogManager::getSingleton().logMessage(e.what());
		throw e;
	}

	ObjectNode = smgr->getRootSceneNode()->createChildSceneNode();
	ObjectNode->attachObject(ObjectItem);
	camera->setNearClipDistance(0.001);
	camera->setFarClipDistance(100);
	/*camera->setPosition(Ogre::Vector3::UNIT_SCALE * 0.0625);
	camera->lookAt({ 0, 0.03125, 0 });*/
	camera->setPosition(Ogre::Vector3::UNIT_SCALE * 2);
	camera->lookAt({ 0, 0, 0 });
	camera->setAutoAspectRatio(true);

	auto light = smgr->createLight();
	smgr->getRootSceneNode()->createChildSceneNode()->attachObject(light);
	light->setType(Ogre::Light::LT_DIRECTIONAL);
	light->setDirection({ -1, -1, -0.5 });
	light->setPowerScale(5);

	while (!window->isClosed())
	{
		ObjectNode->setOrientation(Ogre::Quaternion(Ogre::Degree(float(root->getTimer()->getMilliseconds()) / 10.0f), Ogre::Vector3::NEGATIVE_UNIT_Y));
		root->renderOneFrame();
		Ogre::WindowEventUtilities::messagePump();
	}

	return 0;
}