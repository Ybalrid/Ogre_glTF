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

int main()
{
}
