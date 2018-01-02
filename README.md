# Ogre_glTF
Library to use glTF resources with Ogre 2.1

Currently this is a molsty empty repository as I'm hacking around to get something working. I don't see any reason not to develop stuff in the open, so here it is.

Right now this is not intended to serve as a "plugin" for Ogre, but to be a little support library that permit you to use standard glTF files in an Ogre application.

The goal is to be able to load the geometry, the PBR material and the animations of an object from glTF and use Ogre's classes as if you just got the object as a .mesh from Ogre's resource manager.

Beffore getting something motsly seemless, my first goal is to construct a valid Ogre::Mesh object from a gltf file, load all the textures in the texture manager, and create an HlmsPbs material.

I'm wanting to get this functionality working inside my game engine (Annwvyn, a VR application developement framework that uses Ogre), and specifically I want to be able to use the glTF official Blender exporter.

## Current state

This is a projet under developement. Here's a short todolist beofre this thing will be in an "usable" state:

 - [x] Load mesh infrmation (index and vertex buffer, including vertex coordinates, normal vectors, texture coordinates) and create an Ogre::Mesh out of it via Ogre::MeshManager and Ogre::VaoManager
 - [x] Load Image information from glTF into Ogre::TextureManager
 - [x] Load PBR material definition form glTF and create coresponding Ogre::HlmsPbsDatablock for them. (Ogre call PBR "PBS", more or less)

 At this point, the library will be able to load static geometry into Ogre.

 If this works, we can start tackeling animation data:
 - [ ] Load "skin" information from glTF and create corresponding Ogre::Skeleton for the mesh
 - [ ] Load mesh "target" information and create Ogre "morph" target from them
 - [ ] Load animation information and create animations from them

In parallel, it could be interesting to use glTF as a "scene" loading format. glTF supports multiple scenes, with nodes having parent/child relations and that can have meshes attached to them


## Building

This project uses CMake. The CMake directory is a simple copy of every cmake script shipped in the Ogre SDK, to make things simpler.

To build the project, you need to have Ogre 2.1 build and "installed" somewhere. Windows users may need to set the OGRE_HOME variable.

then, do the folliwng :

```bash
cd build
cmake ..                        #execute CMake while pointing at the parent directory
make
cp -r <path to HLMS> .          #add the Hlms shader code that comes with Ogre
cp <path to ogre plugins>/* .   #add the necessary plugins (RenderSystem_GL3+)
```

On a typical install from the source code on linux, theses path are `/usr/local/share/OGRE/Media/Hlms` and `/usr/local/lib/OGRE/*`

## Notes on third party components

This library is based on tinygltf. https://github.com/syoyo/tinygltf. tinygltf itsefl vendor in some other opensource projects, like stb_image and a json loading library.

tinygltf is an header only library. It is included in this very repository via git submodules.
If you are about to clone this repository, you should use `git clone --recursive`


