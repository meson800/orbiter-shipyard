//Copyright (c) 2015 Christopher Johnstone(meson800) and Benedict Haefeli(jedidia)
//The MIT License - See ../../LICENSE for more info

#include "StackImport.h"


StackImport::StackImport(ImportData *data)
{
	_data = data;
	prepareImportData();
}


StackImport::~StackImport()
{
}



//reads out the currently focused stack from orbiter and prepares the data so it can be created in SE
void StackImport::prepareImportData()
{
	_data->locked = true;
	//get the focused vessel from orbiter
	VESSEL *v = oapiGetFocusInterface();

	//fill a temporary container with the import data 
	std::vector<VesselExport> container;
	readStack(container, v);
	//copy the export data into the ImportData struct
	for (UINT i = 0; i < container.size(); ++i)
	{
		_data->stack.push(container[i]);
	}
	_data->locked = false;
}


/* reads in a stack beginning with the passed vessel (RECURSIVE!)
 * v: vessel to put into stack
 * caller: vessel that called the operation on v 
 * callerportindex: the dockport index on the caller that connects to this vessel
 * callerindex: the index of the calling vessel in the _vessels queue*/
 
void StackImport::readStack(std::vector<VesselExport> &container, VESSEL *v, VESSEL *caller, UINT callerportindex, UINT callerindex)
{
	//add the vessel to the list
	UINT myindex = container.size();
	VesselExport vexport;
	vexport.className = v->GetClassNameA();
	vexport.className += ".cfg";
	vexport.orbitername = v->GetName();
	container.push_back(vexport);

	for (UINT i = 0; i < v->DockCount(); ++i)
	{
		if (v->DockingStatus(i) > 0)
		{
			//found another vessel docked, add it to the queue
			VESSEL* dockedv = oapiGetVesselInterface(v->GetDockStatus(v->GetDockHandle(i)));
			if (dockedv != caller)
			{
				readStack(container, dockedv, v, i, myindex);
			}
			else
			{
				//the caller is docked to this port. mark the connection.
				DockPortExport dexport;
				dexport.myindex = i;
				dexport.dockedToPort = callerportindex;
				dexport.dockedToVessel = callerindex;
				container[myindex].dockports.push(dexport);
			}
		}
	}
}


