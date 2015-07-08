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
		//walk through dockports of vessel
		for (UINT j = 0; j < v->dockingPorts.size(); ++j)
		{
			if (v->dockingPorts[j].docked)
			{
				//get the index of the docked vessel
				int dockedvidx = v->dockingPorts[j].dockedTo.vesselUID;
				//connections are only remembered down the queue. 
				//Vessels appearing earlier in the queue must not know that they're docked to vessels higher up
				if (dockedvidx < i)
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

	//generating name for vessel
	std::stringstream vesselname;
	vesselname << _data->name;
	//first vessel will bear the stack name, following vessels will get a number appended
	if (vessels_processed > 0)
	{
		vesselname << "_" << vessels_processed;
	}
	//get the classname without the file extension, because orbiter wants it that way
	string classname(_vessels.front().className.substr(0, _vessels.front().className.find_last_of(".")));
	//put classname in caps, because IMS wants it that way. 
	//and it won't recognise the class as an IMS module if it's written in small letters. Probably needs glasses...
	//oh, and switch any / with \. because I was kind of stupid when I wrote IMS.
	std::transform(classname.begin(), classname.end(), classname.begin(), ::toupper);
	Helpers::slashreplace(classname);
	// create the vessel, add its handle to the list and get the new interface from orbiter
	OBJHANDLE newvessel = oapiCreateVesselEx(vesselname.str().data(), classname.data(), &vs);
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