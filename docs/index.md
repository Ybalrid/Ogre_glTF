# Ogre_glTF

 - [Doxygen site](./doxygen)
 - [Quickstart Guide](./guide)

Welcome!

This project is a small utility library to use glTF resources with Ogre 2.1 licencied under the terms of the [MIT licence](https://opensource.org/licenses/MIT "MIT Licence on the OSI website").

The goal is to be able to load any valid glTF file into Ogre, and create Ogre meshes with the correct material and animations applied.

This is currently under heavy developement, but as you can see, we are getting somewhere:

![DEMO](./demo.gif)

This library is based on tinygltf. https://github.com/syoyo/tinygltf. tinygltf itself vendor-in some other opensource projects, like stb_image and a json loading library.


## Feature list

 - Load the first declared mesh (or the first mesh of the default scene) of a glTF or glb (binary glTF) file into Ogre
 - Load all the texture and a PBR material into Ogre's High Level Material System (PBS implementation)
 - Create Ogre 2.x Item into your scene manager
 
## Binary releases

There isn't any binaries avaiable, if you want to use the library, you should get the lattest version of the source code and build it yourself (see below)

## Building

First of all, locally clone this repository. It uses git submodules to get it's dependencies, so you just need to do 

```
git clone --recursive https://github.com/Ybalrid/Ogre_glTF
```

This project uses CMake. The CMake directory is a simple copy of every cmake script shipped in the Ogre SDK, to make things simpler.

You should get and install Ogre 2.1 from source on your system, 

To build the project, you need to have Ogre 2.1 build and "installed" somewhere. Windows users may need to set the `OGRE_HOME` variable.

then, do the folliwng (linux) :

```bash
cd build
cmake ..                        #execute CMake while pointing at the parent directory
make                            #build the library and the demo program

#to be able to run the demo program as-is, do the following:
cp -r <path to HLMS> .          #add the Hlms shader code that comes with Ogre
cp <path to ogre plugins>/* .   #add the necessary plugins (RenderSystem_GL3+)
```
On a typical install from Ogre's source code on linux, theses path are `/usr/local/share/OGRE/Media/Hlms` and `/usr/local/lib/OGRE/*`

(windows) :

 - Use cmake-gui to generate a Visual Studio solutution inside the `build` using the same version that you built Ogre with. You probably need to set the `OGRE_HOME` variable.
 - Open the .sln (solution) file into Visual Studio. You'll get 2 projects : `Ogre_glTF` (the DLL) and `Ogre_glTF_TEST` (a test program)
 - To make the test program works, copy inside the "build" directory all the .dll files from Ogre's debug and release binary directories
 - Copy the HLMS libary to the "build" directory

## Known issues

 - There's a problem with loading normal map data with the Direct 3D 11 render system of Ogre [issue #2](https://github.com/Ybalrid/Ogre_glTF/issues/2)
 - There's several little issues with the texture loading. A small refactor would help. See the TODO comments.
 - Library is not "installable" from CMakeLists.txt yet. Users need to get the .dll / .so file accessible to their program, and point their compiler to look for headers the "include" directory
 - Can only load one mesh and it's associated material in a file. Will either load the first one, of the fist node of the default scene, depending if the default scene is set
 - Library only has been tested on an handfull of glTF files. 

## Notes on third party components

tinygltf is an header only library. It is included in this very repository via git submodules.
If you are about to clone this repository, you should use `git clone --recursive`

The library define inside one of it's files the implementation of `tinygltf` and `stb_image`. This shouldn't be an issue and your program using ogre_glTF shouldn't be affected by them in any way.

If you have issues related with them, please open an issue :)
