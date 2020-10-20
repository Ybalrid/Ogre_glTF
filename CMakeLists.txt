cmake_minimum_required(VERSION 3.8)

project(Ogre_glTF)

#to build a library
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INSTALL_PREFIX "./output")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG=1 -D_DEBUG=1") 

if(NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX "_d")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CMake)

include(CMake/OGRE.cmake)
setupOgre( OGRE_SOURCE, OGRE_BINARIES, OGRE_LIBRARIES, FALSE )
include(Ogre_glTF_ConfigTargets)

if(WIN32)
    set( LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin" )
else()
    set( LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}" )
endif()

message("EXECUTABLE_OUTPUT_PATH :" ${EXECUTABLE_OUTPUT_PATH})
option(Ogre_glTF_STATIC "Static build" FALSE)

if (Ogre_glTF_STATIC)
  add_definitions(-DOgre_glTF_STATIC)
  set(Ogre_glTF_LIB_TYPE STATIC)
  set(Ogre_glTF_LIB_SUFFIX "Static")
else ()
  set(Ogre_glTF_LIB_TYPE SHARED)
  set(Ogre_glTF_LIB_SUFFIX "")
endif ()

#Get Ogre from your system. May need to set some variables for Windows folks
#find_package(OGRE COMPONENTS HlmsPbs REQUIRED)
file(GLOB librarySources ./src/*.cpp ./src/private_headers/*.hpp ./include/*.hpp)

add_library(${PROJECT_NAME} ${Ogre_glTF_LIB_TYPE} ${librarySources})

target_compile_definitions(${PROJECT_NAME} PUBLIC Ogre_glTF_DLL_EXPORT_CONFIG_ON)

set_target_properties(${PROJECT_NAME} PROPERTIES 
    OUTPUT_NAME ${PROJECT_NAME}${Ogre_glTF_LIB_SUFFIX}
    #LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${BUILD_TYPE}"
)

target_include_directories( Ogre_glTF PUBLIC
	#Ogre and the physics based high level material system
	${OGRE_INCLUDE_DIRS}
	${OGRE_HlmsPbs_INCLUDE_DIRS}
	${OGRE_HlmsPbsCommon_INCLUDE_DIRS}
	#local include directory
	./include
	./src/private_headers
	./thirdParty/tinygltf/
)

target_link_libraries(Ogre_glTF
	${OGRE_LIBRARIES}
	${OGRE_HlmsPbs_LIBRARIES}
)

add_subdirectory(Samples)

message("OGRE_RenderSystem_GL3Plus_LIBRARY_REL: " ${OGRE_RenderSystem_GL3Plus_LIBRARY_REL})
message("OGRE_BINARY_REL: " ${OGRE_BINARY_REL})
message("OGRE_BINARY_DBG: " ${OGRE_BINARY_DBG})

#file( COPY ${OGRE_BINARY_REL} ${OGRE_BINARY_REL}.${OGRE_VERSION} ${OGRE_RenderSystem_GL3Plus_LIBRARY_REL}.${OGRE_VERSION}
#      DESTINATION  "${CMAKE_BINARY_DIR}/bin")

#installation

install(FILES
./include/Ogre_glTF.hpp
./include/Ogre_glTF_OgrePlugin.hpp
./include/Ogre_glTF_OgreResource.hpp
./include/Ogre_glTF_DLL.hpp
DESTINATION
"include")

install(TARGETS Ogre_glTF DESTINATION "bin")

