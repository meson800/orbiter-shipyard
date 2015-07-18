//Copyright (c) 2015 Christopher Johnstone(meson800) and Benedict Haefeli(jedidia)
//The MIT License - See ../../LICENSE for more info
#include "VesselStackOperations.h"

void VesselStackOperations::splitStack(OrbiterDockingPort* sourcePort)
{
    Log::writeToLog(Log::INFO, "Splitting stack on vessel UID: ", sourcePort->dockedTo.vesselUID, 
        " Port UID: ", sourcePort->dockedTo.portID);
    OrbiterDockingPort* destPort = &(Helpers::getVesselByUID(sourcePort->dockedTo.vesselUID)->
        dockingPorts[sourcePort->dockedTo.portID]);
	
	//reset docked flags	
	sourcePort->docked = false;
	destPort->docked = false;
	//reset dockedTo pointers
	sourcePort->dockedTo.vesselUID = 0;
    sourcePort->dockedTo.portID = 0;
	destPort->dockedTo.vesselUID = 0;
    destPort->dockedTo.portID = 0;
}

std::vector<VesselSceneNode*> VesselStackOperations::copyStack(VesselStack* stack, scene::ISceneManager* smgr)
{
    Log::writeToLog(Log::INFO, "Copying vessel stack: ", stack->toString());
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
    //make a map from old UID to new UID
	std::map<UINT, UINT> vesselUIDTransferMap;
	for (UINT i = 0; i < stack->numVessels(); ++i)
	{
        VesselSceneNode* oldVessel = stack->getVessel(i);
        VesselSceneNode* newVessel = newNodes[i];
        vesselUIDTransferMap[oldVessel->getUID()] = newVessel->getUID();
	}

	//Now, recurse through the docking ports one more time, setting the pointers correctly with the map
	for (UINT i = 0; i < stack->numVessels(); ++i)
	{
		VesselSceneNode* oldVessel = stack->getVessel(i);
		VesselSceneNode* newVessel = newNodes[i];
		for (UINT j = 0; j < oldVessel->dockingPorts.size(); ++j)
		{
			newVessel->dockingPorts[j].docked = oldVessel->dockingPorts[j].docked;
			newVessel->dockingPorts[j].dockedTo.vesselUID = vesselUIDTransferMap[oldVessel->dockingPorts[j].dockedTo.vesselUID];
            newVessel->dockingPorts[j].dockedTo.portID = oldVessel->dockingPorts[j].dockedTo.portID;
		}
	}

	//Finally, return the new vessels we created so they can be registered
	//Bad stuff happens if the new nodes aren't registered (stackEditor can't use the dockport lookup table)
	return newNodes;

}

//Assumes that vessels have been deregistered prior to sending it to this
void VesselStackOperations::deleteStack(VesselStack* stack)
{
    Log::writeToLog(Log::INFO, "Deleting vessel stack: ", stack->toString());
	for (UINT i = 0; i < stack->numVessels(); ++i)
	{
        VesselSceneNode* vessel = stack->getVessel(i);
		vessel->removeAll(); //removeAll only drops children, not the node itself
		vessel->remove(); //remove from scene graph
        vessel->drop(); //We need the extra drop because we called VesselSceneNode, which returns a pointer
	}
}
