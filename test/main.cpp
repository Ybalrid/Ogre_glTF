#include <Ogre.h>
#include <memory>

#include <Ogre_glTF.hpp>


#ifdef _DEBUG
const char RENDER_PLUGIN[] = "RenderSystem_GL3Plus_d";
#else
const char RENDER_PLUGIN[] = "RenderSystem_GL3Plus";
#endif

#ifdef _WIN32
#include <windows.h>
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
int main(int argc, char* argv[])
#endif
{
	auto root = std::make_unique<Ogre::Root>();
	root->loadPlugin(RENDER_PLUGIN);
	root->setRenderSystem(root->getAvailableRenderers().front());
	root->initialise(false);

	auto window = root->createRenderWindow("glTF test!", 800, 600, false, nullptr);
	auto gltf = std::make_unique<Ogre_glTF>();

	while(!window->isClosed())
	{
		Ogre::WindowEventUtilities::messagePump();
	}

	return 0;
}