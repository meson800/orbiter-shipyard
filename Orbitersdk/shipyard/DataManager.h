#pragma once

#include <mutex>

#include "Common.h"
#include "OrbiterMesh.h"
#include "OrbiterDockingPort.h"

struct VesselData
{
	std::string className;
	OrbiterMesh *vesselMesh;
	vector<OrbiterDockingPort> dockingPorts;
	ITexture *vesselImg;
};



class DataManager
{
public:
	DataManager();
	~DataManager();

	OrbiterMesh* GetGlobalMesh(std::string meshName, video::IVideoDriver* driver);
	VesselData* GetGlobalConfig(std::string configName, video::IVideoDriver* driver);
	video::ITexture *GetGlobalImg(std::string imgName, video::IVideoDriver* driver);

private:
	VesselData* LoadVesselData(std::string configFileName, video::IVideoDriver* driver);

	std::mutex meshMutex, configMutex, imgMutex;	//stores mutexes for safe multithreading

	std::map<std::string, OrbiterMesh*> meshMap;		//stores all loaded meshes
	std::map<std::string, VesselData*> cfgMap;		//stores all loaded configs
	std::map<std::string, video::ITexture*> imgMap;	//stores all loaded images
};