//Copyright (c) 2015 Christopher Johnstone(meson800) and Benedict Haefeli(jedidia)
//The MIT License - See ../../LICENSE for more info
#include "orbitersdk.h"
#include <queue>
#include "StackExportStructs.h"
#include "StackExport.h"


StackExport::StackExport(ExportData *data)
{
	_data = data;
	prepareExportData();
	vessels_processed = 0;
}

StackExport::~StackExport()
{
}


void StackExport::prepareExportData()
{
	//walk through stack
	for (UINT i = 0; i < _data->stack->getStackSize(); ++i)
	{
		VesselSceneNode* v = _data->stack->getVessel(i);
		VesselExport newv;
		newv.className = v->getClassName();
		newv.orbitername = v->getOrbiterName();
		newv.vesselid = v->getUID();

		//walk through dockports of vessel
		for (UINT j = 0; j < v->dockingPorts.size(); ++j)
		{
			if (v->dockingPorts[j].docked)
			{
				//get the index of the docked vessel
				int dockedvidx = _data->stack->getIndexOfVessel(Helpers::getVesselByUID(v->dockingPorts[j].dockedTo.vesselUID));
				//connections are only remembered down the queue. 
				//Vessels appearing earlier in the queue must not know that they're docked to vessels higher up
				if (dockedvidx < (int)i)
				{ 
					DockPortExport newd;
					newd.dockedToVessel = dockedvidx;
                    newd.dockedToPort = v->dockingPorts[j].dockedTo.portID;
					newd.myindex = j;
					newv.dockports.push(newd);
				}
			}
		}
		_vessels.push(newv);
	}
}


bool StackExport::advanceQueue()
{
	//initialise VESSELSTATUS for new vessel (taken from ScenarioEditor
	VESSELSTATUS2 vs;
	memset(&vs, 0, sizeof(vs));
	vs.version = 2;
	vs.rbody = oapiGetGbodyByName("Earth");
	if (!vs.rbody) vs.rbody = oapiGetGbodyByIndex(0);
	double rad = 1.1 * oapiGetSize(vs.rbody);
	double vel = sqrt(GGRAV * oapiGetMass(vs.rbody) / rad);
	vs.rpos = _V(rad, 0, 0);
	vs.rvel = _V(0, 0, vel);

	OBJHANDLE newvessel = oapiGetVesselByName((char*)_vessels.front().orbitername.data());	//an ugly cast for sure, but in my expierience orbiter doesn't do anything untowards with it
	
	if (newvessel == NULL)
	//vessel does not exist in orbiter, create it
	{
		//generating name for vessel
		std::stringstream vesselname;
		vesselname << _data->name;

		//first vessel will bear the stack name, following vessels will get one or more numbers appended
		//the important thing is that we don't have name duplication in orbiter
		while (oapiGetVesselByName((char*)vesselname.str().data()) != NULL)
		{
			vesselname << "_" << vessels_processed;
		}
		//set the orbiter name for the VesselSceneNode so it can identify with the orbiter vessel without re-importing first
		Helpers::getVesselByUID(_vessels.front().vesselid)->setOrbiterName(vesselname.str());

		//get the classname without the file extension, because orbiter wants it that way
		string classname(_vessels.front().className.substr(0, _vessels.front().className.find_last_of(".")));
		//put classname in caps, because IMS wants it that way. 
		//and it won't recognise the class as an IMS module if it's written in small letters. Probably needs glasses...
		//oh, and switch any / with \. because I was kind of stupid when I wrote IMS.
		std::transform(classname.begin(), classname.end(), classname.begin(), ::toupper);
		Helpers::slashreplace(classname);
		// create the vessel, add its handle to the list and get the new interface from orbiter
		newvessel = oapiCreateVesselEx(vesselname.str().data(), classname.data(), &vs);
	}
	else
	//vessel DOES exist in orbiter, undock it
	{
		oapiGetVesselInterface(newvessel)->Undock(ALLDOCKS);
	}

	_createdvessels.push_back(newvessel);
	VESSEL *v = oapiGetVesselInterface(newvessel);

	//walk through dockports and dock. The way the structure was created ensures that no vessel will try to dock to one that has not been created yet
	while (_vessels.front().dockports.size() > 0)
	{
		v->Dock(_createdvessels[_vessels.front().dockports.front().dockedToVessel],
			_vessels.front().dockports.front().myindex,
			_vessels.front().dockports.front().dockedToPort, 2);
		
		_vessels.front().dockports.pop();
	}

	if (vessels_processed == 0)
	{
		//set focus and camera to first vessel in stack
		oapiSetFocusObject(_createdvessels[0]);
		oapiCameraAttach(_createdvessels[0], 1);
	}

	_vessels.pop();
	vessels_processed++;

	return _vessels.size() == 0;
}
