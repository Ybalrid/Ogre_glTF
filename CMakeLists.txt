cmake_minimum_required(VERSION 3.8)

project(Ogre_glTF)

#to build a library
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INSTALL_PREFIX "./output")
set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG=1 -D_DEBUG=1") 

if(NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX "_d")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CMake)

#get Ogre from your system. May need to set some variable for Windows folks
find_package(OGRE COMPONENTS HlmsPbs REQUIRED)

file(GLOB librarySources ./src/*.cpp ./src/private_headers/*.hpp ./include/*.hpp)
file(GLOB testSources ./test/*.cpp ./test/*.hpp ./include/*.hpp)
file(GLOB pluginTestSources ./pluginTest/*.cpp ./pluginTest/*.hpp ./include/*.hpp)

add_library(Ogre_glTF SHARED ${librarySources})
#add_library(Ogre_glTF_static STATIC ${librarySources})

target_compile_definitions(Ogre_glTF PUBLIC Ogre_glTF_DLL_EXPORT_CONFIG_ON)

if(MSVC)
	add_executable(Ogre_glTF_TEST WIN32 ${testSources})
	add_executable(Ogre_gltf_PluginTest WIN32 ${pluginTestSources})
	#add_executable(Ogre_glTF_TEST_static WIN32 ${testSources})
else()
	add_executable(Ogre_glTF_TEST ${testSources})
	add_executable(Ogre_gltf_PluginTest ${pluginTestSources})
	#add_executable(Ogre_glTF_TEST_static ${testSources})
endif(MSVC)

target_include_directories( Ogre_glTF PUBLIC
	#Ogre and the physics based high level material system
	${OGRE_INCLUDE_DIRS}
	${OGRE_HlmsPbs_INCLUDE_DIRS}

	#local include directory
	./include
	./src/private_headers
	./thirdParty/tinygltf/
)

#target_include_directories( Ogre_glTF_static PUBLIC
#	#Ogre and the physics based high level material system
#	${OGRE_INCLUDE_DIRS}
#	${OGRE_HlmsPbs_INCLUDE_DIRS}

#	#local include directory
#	./include
#	./src/private_headers
#	./thirdParty/tinygltf/
#)

target_include_directories( Ogre_glTF_TEST PUBLIC
	#Ogre and the physics based high level material system
	${OGRE_INCLUDE_DIRS}
	${OGRE_HlmsPbs_INCLUDE_DIRS}

	#local include directory
	./include
)


# target_include_directories( Ogre_glTF_TEST_static PUBLIC
#	#Ogre and the physics based high level material system
#	${OGRE_INCLUDE_DIRS}
#	${OGRE_HlmsPbs_INCLUDE_DIRS}
#
#	#local include directory
#	./include
#)

target_include_directories(Ogre_gltf_PluginTest PUBLIC
	${OGRE_INCLUDE_DIRS}
	${OGRE_HlmsPbs_INCLUDE_DIRS}
	./include
)

target_link_libraries(Ogre_glTF
	${OGRE_LIBRARIES}
	${OGRE_HlmsPbs_LIBRARIES}
)

target_link_libraries(Ogre_glTF_TEST
	${OGRE_LIBRARIES}
	${OGRE_HlmsPbs_LIBRARIES}

    #the library we just built
	Ogre_glTF
)

target_link_libraries(Ogre_gltf_PluginTest
	${OGRE_LIBRARIES}
	${OGRE_HlmsPbs_LIBRARIES}
)

add_dependencies(Ogre_gltf_PluginTest Ogre_glTF)

#target_link_libraries(Ogre_glTF_TEST_static
#	${OGRE_LIBRARIES}
#	${OGRE_HlmsPbs_LIBRARIES}
#
#    #the library we just built
#	Ogre_glTF_static
#)

#installation

install(TARGETS Ogre_glTF DESTINATION "bin")
install(FILES
./include/Ogre_glTF.hpp
./include/Ogre_glTF_OgrePlugin.hpp
./include/Ogre_glTF_OgreResource.hpp
./include/Ogre_glTF_DLL.hpp
DESTINATION
"include")
