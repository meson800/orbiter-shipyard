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

std::vector<VesselSceneNode*> VesselStackOperations::copyStack(VesselStack* stack, scene::ISceneManager* smgr)
{
	//Ugh, too bad ISceneNode doesn't implement a copy constructor, so we have to manually implement :(

	//Start by copying every node
	vector<VesselSceneNode*> newNodes;
	for (UINT i = 0; i < stack->numVessels(); ++i)
	{
		VesselSceneNode* oldVessel = stack->getVessel(i);
		VesselSceneNode* newVessel = new VesselSceneNode(oldVessel->returnVesselData(), smgr->getRootSceneNode(), smgr, VESSEL_ID);
		newNodes.push_back(newVessel);

		//Now copy position and rotation of node
		oldVessel->updateAbsolutePosition(); //Not sure if this line is needed...
		newVessel->setPosition(oldVessel->getPosition());
		newVessel->setRotation(oldVessel->getRotation());
	}

	//Now we have to set up the docking ports correctly.
	//The hardest part is swapping the pointers so each docking port's dockedTo pointer is set correctly.
	//To do this, we have to get a pointer to pointer map going for each docking port
	std::map<OrbiterDockingPort*, OrbiterDockingPort*> dockingPortPointerMap;
	for (UINT i = 0; i < stack->numVessels(); ++i)
	{
		VesselSceneNode* oldVessel = stack->getVessel(i);
		VesselSceneNode* newVessel = newNodes[i];
		for (UINT j = 0; j < oldVessel->dockingPorts.size(); ++j)
		{
			dockingPortPointerMap[&(oldVessel->dockingPorts[j])] = &(newVessel->dockingPorts[j]);
		}
	}

	//Now, recurse through the docking ports one more time, setting the pointers correctly with the map
	for (UINT i = 0; i < stack->numVessels(); ++i)
	{
		VesselSceneNode* oldVessel = stack->getVessel(i);
		VesselSceneNode* newVessel = newNodes[i];
		for (UINT j = 0; j < oldVessel->dockingPorts.size(); ++j)
		{
			newVessel->dockingPorts[j].docked = oldVessel->dockingPorts[j].docked;
			newVessel->dockingPorts[j].dockedTo = dockingPortPointerMap[oldVessel->dockingPorts[j].dockedTo];
		}
	}

	//Finally, return the new vessels we created so they can be registered
	//Bad stuff happens if the new nodes aren't registered (shipyard can't use the dockport lookup table)
	return newNodes;

}

//Assumes that vessels have been deregistered prior to sending it to this
void VesselStackOperations::deleteStack(VesselStack* stack)
{
	for (UINT i = 0; i < stack->numVessels(); ++i)
	{
		stack->getVessel(i)->removeAll(); //removeAll only drops children, not the node itself
		stack->getVessel(i)->remove();
	}
}