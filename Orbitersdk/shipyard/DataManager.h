#pragma once

#include "Common.h"
#include "OrbiterMesh.h"
#include "OrbiterDockingPort.h"

struct VesselData
{
	string className;
	OrbiterMesh *vesselMesh;
	vector<OrbiterDockingPort> dockingPorts;
	ITexture *vesselImg;
};



class DataManager
{
public:
	DataManager();
	~DataManager();

	OrbiterMesh* GetGlobalMesh(string meshName, video::IVideoDriver* driver);
	VesselData* GetGlobalConfig(string configName, video::IVideoDriver* driver);
	video::ITexture *GetGlobalImg(string imgName, video::IVideoDriver* driver);

private:
	VesselData* LoadVesselData(string configFileName, video::IVideoDriver* driver);

	map<string, OrbiterMesh*> meshMap;		//stores all loaded meshes
	map<string, VesselData*> cfgMap;		//stores all loaded configs
	map<string, video::ITexture*> imgMap;	//stores all loaded images
};