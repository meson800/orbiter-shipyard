//Copyright (c) 2015 Christopher Johnstone(meson800) and Benedict Haefeli(jedidia)
//The MIT License - See ../../LICENSE for more info
//structs neccessary for exporting data to orbiter.
//only include in orbiter_main.cpp

#pragma once
#include <queue>
//#include "OrbiterDockingPort.h"
#include "VesselStack.h"

//structs that contain the neccessary data to export a stack into orbiter
struct DockPortExport
{
	int dockedToVessel = -1;	//index of vessel docked to this port (in StackExport::vessels queue)
	int dockedToPort = -1;		//index of docking port on docked vessel
	int myindex = -1;			//index of this dockport on its vessel
};

struct VesselExport
{
	std::string className;
	std::queue<DockPortExport> dockports;
	UINT vesselid;
	std::string orbitername;
};

struct ExportData
{
	bool locked = false;
	bool exporting = false;						//true while orbiter export is running
	VesselStack* stack = NULL;					//pointer to stack to be exported
	std::string name;							//base name of the stack
};

struct ImportData
{
	bool locked = false;
	bool importing = false;
	std::queue<VesselExport> stack;
};
