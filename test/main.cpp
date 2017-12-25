#include <Ogre.h>
#include <memory>

#include <Ogre_glTF.hpp>

#ifdef _WIN32
#include <windows.h>
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
int main(int argc, char* argv[])
#endif
{
	auto root = std::make_unique<Ogre::Root>();

	auto gltf = std::make_unique<Ogre_glTF>();

	return 0;
}