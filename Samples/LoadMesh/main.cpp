#include "SamplesCommon.h"
#include "OgreMeshManager.h"
#include "OgreMeshManager2.h"
#include "OgreMesh2.h"


int main()
{
#ifdef Ogre_glTF_STATIC
	// Must instantiate before Root so that it'll be destroyed afterwards. 
	// Otherwise we get a crash on Ogre::Root::shutdownPlugins()
#if __linux__
    auto glPlugin = std::make_unique<Ogre::GL3PlusPlugin>();
#endif
#endif

	//Init Ogre
	auto root = std::make_unique<Ogre::Root>();
	Ogre::LogManager::getSingleton().setLogDetail(Ogre::LoggingLevel::LL_BOREME);

/*
#ifdef Ogre_glTF_STATIC
#if __linux__
    root->installPlugin(glPlugin.get());
#endif
#else
	root->loadPlugin(GL_RENDER_PLUGIN);
#ifdef _WIN32
	root->loadPlugin(D3D11_RENDER_PLUGIN);
#endif
#endif
*/
	if(!root->showConfigDialog()) return -1;
	Ogre::Window* window = root->initialise(true, "Gltf loader sample");
	
	auto smgr = root->createSceneManager(Ogre::ST_GENERIC, 2);
	auto camera = smgr->createCamera("cam");
	
	//Setup rendering pipeline
	auto compositor			   = root->getCompositorManager2();
	const char workspaceName[] = "workspace0";
	compositor->createBasicWorkspaceDef(workspaceName, Ogre::ColourValue { 0.2f, 0.3f, 0.4f });
	auto workspace = compositor->addWorkspace(smgr, window->getTexture(), camera, workspaceName, true);

	// see Ogre.cmake script
	DeclareHlmsLibrary("../Data");
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("../../Media/gltfFiles.zip", "Zip", "glTf");
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("../Data/DebugPack.zip", "Zip", "General");
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups(true);

	auto gltf = std::make_unique<Ogre_glTF::glTFLoader>();
	Ogre::SceneNode* objectNode = nullptr;

	try
	{
		auto adapter = gltf->loadFromFileSystem("../../Media/damagedHelmet/damagedHelmet.gltf");
		//auto adapter = gltf->loadGlbResource("CesiumMan.glb");
		//auto adapter = gltf->loadGlbResource("blob.glb");
		//auto adapter = gltf->loadGlbResource("BrainStem.glb");
		//auto adapter = gltf->loadGlbResource("CesiumMan.glb");
		//auto adapter = gltf->loadGlbResource("Corset.glb");
		//auto adapter = gltf->loadGlbResource("gizmo.glb");
		//auto adapter = gltf->loadGlbResource("marker.glb");
		//auto adapter = gltf->loadGlbResource("Monster.glb");
		objectNode   = adapter.getFirstSceneNode(smgr);

		// On a scene with multiple root objects `loadMainScene` can be used to load all objects.
		/*
		auto root = smgr->getRootSceneNode();
		adapter.loadMainScene(root, smgr);
		auto childIt = root->getChildIterator();
		while(childIt.hasMoreElements())
		{
			auto child = childIt.getNext();
			if(child->getName() == "UnityGlTF_root")
			{
				objectNode = static_cast<Ogre::SceneNode*>(child);
				break;
			}
		}*/
	}
	catch(std::exception& e)
	{
		Ogre::LogManager::getSingleton().logMessage(e.what());
		return -1;
	}

	camera->setNearClipDistance(0.001f);
	camera->setFarClipDistance(100);
	camera->setPosition(2.5f, 0.6f, 2.5f);
	camera->lookAt({ 0, -0.1f, 0 });
	camera->setAutoAspectRatio(true);
	
	Ogre::Light* light;

	light = smgr->createLight();
	smgr->getRootSceneNode()->createChildSceneNode()->attachObject(light);
	light->setType(Ogre::Light::LT_DIRECTIONAL);
	light->setDirection(Ogre::Vector3 { -1, -1, -0.5f });
	light->setPowerScale(5);
		
	light = smgr->createLight();
	smgr->getRootSceneNode()->createChildSceneNode()->attachObject(light);
	light->setType(Ogre::Light::LT_DIRECTIONAL);
	light->setDirection(Ogre::Vector3 { +1, +1, +0.5f });
	light->setPowerScale(5);
	
	auto last = root->getTimer()->getMilliseconds();
	auto now  = last;
	Ogre::Real accumulator = 0;

	while(!window->isClosed())
	{
		now = root->getTimer()->getMilliseconds();
		accumulator += (now - last) / 1000.0f;
		last = now;

		objectNode->setOrientation(Ogre::Quaternion(Ogre::Radian(accumulator), Ogre::Vector3::UNIT_Y));
		root->renderOneFrame();
		Ogre::WindowEventUtilities::messagePump();

	}

	return 0;
}
