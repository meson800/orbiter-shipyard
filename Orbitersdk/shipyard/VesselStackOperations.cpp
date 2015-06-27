#include "VesselStackOperations.h"

void VesselStackOperations::splitStack(OrbiterDockingPort* sourcePort)
{
	OrbiterDockingPort* destPort = sourcePort->dockedTo;
	
	//reset docked flags	
	sourcePort->docked = false;
	destPort->docked = false;
	//reset dockedTo pointers
	sourcePort->dockedTo = 0;
	destPort->dockedTo = 0;
}