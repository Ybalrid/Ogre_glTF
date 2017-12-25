cmake_minimum_required(VERSION 3.8)

project(Ogre_glTF)

#to build a library
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE) 

#add a preprocessor definition for windows for selecting the export directive for DLL with a macro
if(MSVC)
	add_definitions(-DDLLDIR_EX)
endif(MSVC)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CMake)

#get Ogre from your system. May need to set some variable for Windows folks
find_package(OGRE COMPONENTS HlmsPbs REQUIRED)


include_directories(
	#Ogre and the physics based high level material system
	${OGRE_INCLUDE_DIRS}
	${OGRE_HlmsPbs_INCLUDE_DIRS}

	#local include directory
	./include
	./thirdParty/tinygltf/
)

file(GLOB librarySources ./src/*.cpp ./include/*.hpp)
file(GLOB testSources ./test/*.cpp ./test/*.hpp ./include/*.hpp)

add_library(Ogre_glTF SHARED ${librarySources})


if(MSVC)
add_executable(Ogre_glTF_TEST WIN32 ${testSources})
else()
add_executable(Ogre_glTF_TEST ${testSources})
endif(MSVC)

target_link_libraries(Ogre_glTF
	${OGRE_LIBRARIES}
	${OGRE_HlmsPbs_LIBRARIES}
)

target_link_libraries(Ogre_glTF_TEST
	${OGRE_LIBRARIES}
	${OGRE_HlmsPbs_LIBRARIES}

	Ogre_glTF

	)

if(MSVC)
	set_target_properties(Ogre_glTF_TEST PROPERTIES LINK_FLAG_RELEASE "/SUBSYSTEM:WINDOWS")
	set_target_properties(Ogre_glTF_TEST PROPERTIES LINK_FLAG_DEBUG "/SUBSYSTEM:WINDOWS")
endif(MSVC)
