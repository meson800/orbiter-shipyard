#include "VesselStack.h"

VesselStack::VesselStack(VesselSceneNode* startingVessel)
{
	//recurse through with the helper, start with a null pointer, so any docked port works
	createStackHelper(startingVessel, 0);
}

//recursive function to init a vessel stack
void VesselStack::createStackHelper(VesselSceneNode* startingVessel, OrbiterDockingPort* fromPort)
{
	//recurse through the docking ports, through docked vessels
	//if the nodes doesn't contain this vessel yet, add it
	if (std::find(nodes.begin(), nodes.end(), startingVessel) == nodes.end())
	{
		nodes.push_back(startingVessel);
	}

	//base case if there are no occupied docking ports except fromPort
	//recurse if there is an occupied docking port not equal to fromPort

	for (unsigned int i = 0; i < startingVessel->dockingPorts.size(); i++)
	{
		//if we are docked to a port that is not fromPort, recurse into it
		if (startingVessel->dockingPorts[i].docked && startingVessel->dockingPorts[i].dockedTo != fromPort)
			createStackHelper(startingVessel->dockingPorts[i].dockedTo->parent,
				&(startingVessel->dockingPorts[i]));
	}

	//we must be in the base case!
	return;
}