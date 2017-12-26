#pragma once

#include <memory>
#include <Ogre.h>

#ifdef _WIN32
#ifdef DLLDIR_EX
#define Ogre_glTF_EXPORT __declspec(dllexport)
#else
#define Ogre_glTF_EXPORT __declspec(dllimport)
#endif
#else
#define Ogre_glTF_EXPORT //dummy
#endif

class Ogre_glTF;

class Ogre_glTF_EXPORT Ogre_glTF_adapter
{
	friend class Ogre_glTF;

	///Private constructor. Ogre_glTF act as a factory for theses object.
	///This will aslo initialize the "pimpl" structure
	Ogre_glTF_adapter();

	///opaque content of the class
	struct impl;

	///pointer to implementation
	std::unique_ptr<Ogre_glTF_adapter::impl> pimpl;

public:
	///This clear the pimpl structure
	~Ogre_glTF_adapter();

	///Deleted copy constructor : non copyable class
	Ogre_glTF_adapter(const Ogre_glTF_adapter&) = delete;
	///Deleted asignment constructor : non copyable class
	Ogre_glTF_adapter& operator=(const Ogre_glTF_adapter&) = delete;

	///Construct an item for this object
	Ogre::Item* getItem() const;

	///Move constructor : object is movable
	Ogre_glTF_adapter(Ogre_glTF_adapter&& other) noexcept;

	bool isOk() const;
};

class Ogre_glTF_EXPORT Ogre_glTF
{
public:
	Ogre_glTF();
	//non copiable
	Ogre_glTF(const Ogre_glTF&) = delete;
	Ogre_glTF& operator=(const Ogre_glTF&) = delete;
	Ogre_glTF_adapter loadFile(const std::string& path) const;

private:
};