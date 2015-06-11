#pragma once

#include <mutex>
#include <thread>

#include "Common.h"
#include "SE_ImsData.h"
#include "OrbiterMesh.h"
#include "OrbiterDockingPort.h"

class SE_PhotoStudio;

struct ToolboxData
//stores only info useful to the toolbox-namely the config file and the toolbox image
//so we can speed up initial loading through background loading
{
	std::string configFileName;
	video::ITexture* toolboxImage = NULL;
	ImsData *imsData = NULL;
};

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
	ToolboxData* GetGlobalToolboxData(std::string configName, video::IVideoDriver* driver);
	video::ITexture *GetGlobalImg(std::string imgname, std::string configname, video::IVideoDriver* driver);
	void Initialise(IrrlichtDevice *device);

private:
	VesselData* LoadVesselData(std::string configFileName, video::IVideoDriver* driver);

	std::mutex meshMutex, configMutex, toolboxMutex, imgMutex;	//stores mutexes for safe multithreading

	std::map<std::string, OrbiterMesh*> meshMap;		//stores all loaded meshes
	std::map<std::string, ToolboxData*> toolboxMap;	//stores all loaded toolbox data
	std::map<std::string, VesselData*> cfgMap;		//stores all loaded configs
	std::map<std::string, video::ITexture*> imgMap;	//stores all loaded images
	SE_PhotoStudio *photostudio;
};