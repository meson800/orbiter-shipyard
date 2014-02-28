#include "StackFunctions.h"

void StackFunctions::createStack(VesselSceneNode* startingVessel, vesselStack& stack)
{
	//recurse through with the helper, start with a null pointer, so any docked port works
	createStackHelper(startingVessel, 0, stack);
}

void StackFunctions::createStackHelper(VesselSceneNode* startingVessel, OrbiterDockingPort* fromPort, vesselStack& stack)
{
	//recurse through the docking ports, through docked vessels
	//if the stack doesn't contain this vessel yet, add it
	if (std::find(stack.begin(), stack.end(), startingVessel) == stack.end())
	{
		stack.push_back(startingVessel);
	}

	//base case if there are no occupied docking ports except fromPort
	//recurse if there is an occupied docking port not equal to fromPort

	for (unsigned int i = 0; i < startingVessel->dockingPorts.size(); i++)
	{
		//if we are docked to a port that is not fromPort, recurse into it
		if (startingVessel->dockingPorts[i].docked && startingVessel->dockingPorts[i].dockedTo != fromPort)
			createStackHelper(startingVessel->dockingPorts[i].dockedTo->parent,
				&(startingVessel->dockingPorts[i]), stack);
	}

	//we must be in the base case!
	return;

}