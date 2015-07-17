#include "VesselStack.h"

VesselStack::VesselStack(VesselSceneNode* startingVessel)
{
	//recurse through with the helper
	createStackHelper(startingVessel);
	issnaped = false;
}

void VesselStack::changeDockingPortVisibility(bool showEmpty, bool showDocked)
{
		for (unsigned int i = 0; i < nodes.size(); i++)
		nodes[i]->changeDockingPortVisibility(showEmpty, showDocked, true);
}

int VesselStack::numVessels()
{
	return nodes.size();
}

VesselSceneNode* VesselStack::getVessel(int index)
{
	if (index >= nodes.size())
		return 0;
	return nodes[index];
}

void VesselStack::rotateStack(core::vector3df relativeRot)
{
	rotateStack(core::quaternion(relativeRot * core::DEGTORAD));
}

/*void VesselStack::rotateStackAroundVessel(core::vector3df relativeRot, VesselSceneNode *vessel)
//rotates the stack around a given vessel (which it must contain!)
// relativeRot: 
{

}*/

void VesselStack::rotateStack(core::quaternion relativeRot)
{
	//simplified by only rotating the first vessel in the stack (the one the stack was selected with) and just snapping the rest of the stack to it.
	//prevents the stack from running away when being rotated.

	core::quaternion thisNodeRotation = core::quaternion(nodes[0]->getRotation() * core::DEGTORAD);
	//rotate this sucker
	thisNodeRotation = thisNodeRotation * relativeRot;
	//set the rotation
	core::vector3df eulerRotation;
	thisNodeRotation.toEuler(eulerRotation);
	nodes[0]->setRotation(eulerRotation * core::RADTODEG);
	nodes[0]->updateAbsolutePosition();
	snapStack(0);

	//use the force, erm, quaternoins to rotate each node in the stack to avoid gimbal lock
/*	for (unsigned int i = 0; i < nodes.size(); i++)
	{
		core::quaternion thisNodeRotation = core::quaternion(nodes[i]->getRotation() * core::DEGTORAD);
		//rotate this sucker
		thisNodeRotation = thisNodeRotation * relativeRot;
		//set the rotation
		core::vector3df eulerRotation;
		thisNodeRotation.toEuler(eulerRotation);
		nodes[i]->setRotation(eulerRotation * core::RADTODEG);
	}

	//now that the nodes are rotated into the correct directions, move them so they line up correctly
	//first, find the center
	core::aabbox3d<f32> overallBox;
	for (unsigned int i = 0; i < nodes.size(); i++)
		overallBox.addInternalBox(nodes[i]->getTransformedBoundingBox());
	core::vector3df center = overallBox.getCenter();
	//now, compare each node's position to the center, rotate the relative position by the overall rotation, and translate it
	for (unsigned int i = 0; i < nodes.size(); i++)
	{
		nodes[i]->updateAbsolutePosition();
		core::vector3df relativePos = nodes[i]->getAbsolutePosition() - center;
		core::vector3df rotatedPos = relativeRot * relativePos;
		nodes[i]->setPosition(nodes[i]->getPosition() + (rotatedPos - relativePos));
	}*/
}

void VesselStack::setMoveReference(core::vector3df refPos)
{
	//set the reference
	moveReference = refPos;
	//set the inital positions of each node
	previousPositions.clear();
	for (unsigned int i = 0; i < nodes.size(); i++)
	{
		previousPositions.push_back(nodes[i]->getPosition());
	}
}

void VesselStack::moveStackRelative(core::vector3df movePos)
{
	//add the move pos to the move reference, then just use normal move stack referenced
	moveStackReferenced(movePos + currentStackLocation);
}

void VesselStack::moveStackReferenced(core::vector3df movePos)
{
	//We have to do this system from the reference point because of the way snapping is handled
	//If this just added a relative position to each of the nodes, a small mouse movement would never
	//be able to move the node out of the snapping radius, resulting in permenant snapping.
	//This way always moves the node to a position relative to the initial position, making snapping work

	if (issnaped)
		return;

	//check to make sure that we have the same amount of positions in previousPositions than we have nodes
	if (nodes.size() != previousPositions.size())
	{
        Log::writeToLog(Log::WARN, "Tried to move a vessel stack without setting up previousPositions");
		return;
	}

	//loop over, setting relative position
	for (unsigned int i = 0; i < nodes.size(); i++)
	{
		nodes[i]->setPosition(previousPositions[i] + (movePos - moveReference));
	}

	//set current location, in "move-referenced" local coords
	currentStackLocation = movePos;

}

void VesselStack::unSnap(core::vector3df refPos)
{
	if (issnaped)
	{
		setMoveReference(refPos);
		issnaped = false;
	}
}

bool VesselStack::isSnaped()
{
	return issnaped;
}

void VesselStack::checkForSnapping(VesselSceneNode* vessel, ISceneNode* dockportnode, bool dock)
{
	if (issnaped && !dock)
		return;

	//find which port on the target the node belongs to
	int targetdock = -1;
	for (UINT i = 0; i < vessel->dockingPorts.size(); ++i)
	{
		if (!vessel->dockingPorts[i].docked && dockportnode == vessel->dockingPorts[i].portNode)
		{
			targetdock = i;
			break;
		}
	}
	if (targetdock == -1)
	//the dockingport on the target is already occupied
	{
		return;
	}

	//get the dockport on this stack that is closest to the snapping port
	int closestvessel = -1;
	int closestport = -1;
	float closestdist = 99999999999;
	for (UINT i = 0; i < nodes.size(); ++i)
	{
		VesselSceneNode* v = nodes[i];
		for (UINT j = 0; j < v->dockingPorts.size(); ++j)
		{
			if (!v->dockingPorts[j].docked)
			{
				//get the distance between the ports
				v->dockingPorts[j].portNode->updateAbsolutePosition();

				float dist = dockportnode->getAbsolutePosition().getDistanceFrom(v->dockingPorts[j].portNode->getAbsolutePosition());
				if (dist < closestdist)
					//this one's closer, mark it as the closest so far
				{
					closestvessel = i;
					closestport = j;
					closestdist = dist;
				}
			}
		}
	}
	
	if (closestvessel != -1 && closestport != -1)
	{
		//we have a valid vessel in the stack and a valid dockport, let's snap
		snapStack(closestvessel, closestport, &vessel->dockingPorts[targetdock]);
		if (dock)
		{
			nodes[closestvessel]->dock(nodes[closestvessel]->dockingPorts[closestport], vessel->dockingPorts[targetdock]);
		}
	}
}


//snaps the passed vessel with the passed port to the target port, and snaps every vessel in the stack to the ports they're docked to
//srcvesselidx: index of the vessel in this stack causing the snap
//srcdockidx: index of the dockport causing the snap
//tgtport: the OrbiterDockingPort the stack is snapping to
void VesselStack::snapStack(int srcvesselidx, int srcdockportidx, OrbiterDockingPort *tgtport)
{
	VesselSceneNode *srcvessel = nodes[srcvesselidx];
	vector<VesselSceneNode*> snapped_in_last_pass;							//will remember the vessels processed in the last pass
	vector<VesselSceneNode*> hasSnapped = snapped_in_last_pass;				//will contain all vessels that have already snapped

	if (tgtport != NULL && srcdockportidx != -1)
	//a target port has been defined, snap the passed vessel to it
	{
		srcvessel->snap(srcvessel->dockingPorts[srcdockportidx], *tgtport);
		hasSnapped.push_back(tgtport->parent);									//mark the vessel we snaped to, or the algorithm will spill over into the neighbouring stack
	}

	//continue to snap the rest of the stack to the source vessel
	snapped_in_last_pass.push_back(srcvessel);								//this is where we start from


	while (snapped_in_last_pass.size() != 0)									//if there's no vessels snapped in a pass, the algorithm is complete
	{
		vector<VesselSceneNode*> snapped_in_this_pass;						//will remember the vessels snapped during this pass

		for (vector<VesselSceneNode*>::iterator it = snapped_in_last_pass.begin(); it != snapped_in_last_pass.end(); ++it)
		//loop through all vessels snapped in last pass
		{
			VesselSceneNode * i = (*it);
			for (vector<OrbiterDockingPort>::iterator jt = i->dockingPorts.begin(); jt != i->dockingPorts.end(); ++jt)
			//loop through all dockports, snap all connected that aren't snapped already
			{
				OrbiterDockingPort j = (*jt);
				if (j.docked && std::find(hasSnapped.begin(), hasSnapped.end(), Helpers::getVesselByUID(j.dockedTo.vesselUID)) == hasSnapped.end())
				//if the dockport has a connection and the connected vessel hasn't snapped already
				{
                    VesselSceneNode* dockedToVessel = Helpers::getVesselByUID(j.dockedTo.vesselUID);
					dockedToVessel->snap(dockedToVessel->dockingPorts[j.dockedTo.portID], j);
					snapped_in_this_pass.push_back(dockedToVessel);
					hasSnapped.push_back(dockedToVessel);
				}
			}
		}
		snapped_in_last_pass = snapped_in_this_pass;
	}

	issnaped = true;
}

//recursive function to init a vessel stack
void VesselStack::createStackHelper(VesselSceneNode* startingVessel)
{
	//recurse through the docking ports, through docked vessels

	//if the nodes doesn't contain this vessel yet, add it and recurse through the nodes
	if (std::find(nodes.begin(), nodes.end(), startingVessel) == nodes.end())
	{
		nodes.push_back(startingVessel);

		for (UINT i = 0; i < startingVessel->dockingPorts.size(); ++i)
		{
			//store all dockingport nodes in the stack. It could occur that the stack tries to dock with itself otherwise
			dockportnodes.push_back(startingVessel->dockingPorts[i].portNode);
            //and recurse into other vessels if docked
            if (startingVessel->dockingPorts[i].docked)
                createStackHelper(Helpers::getVesselByUID(startingVessel->dockingPorts[i].dockedTo.vesselUID));
		}
	}

	//base case if we have already added this vessel to the list

	//we must be in the base case!
	return;
}


bool VesselStack::isVesselInStack(VesselSceneNode* vessel)
{
	return std::find(nodes.begin(), nodes.end(), vessel) != nodes.end();
}


//returns the index of the vessel in the stacks nodes vector. returns -1 if vessel is not in stack
int VesselStack::getIndexOfVessel(VesselSceneNode* vessel)
{
	int idx = -1;
	vector<VesselSceneNode*>::iterator it = std::find(nodes.begin(), nodes.end(), vessel);
	if (it != nodes.end())
	{
		idx = it - nodes.begin();
	}
	return idx;
}

UINT VesselStack::getStackSize()
{
	return nodes.size();
}

//makes the first node in the stack transparent and shows connected dockports
void VesselStack::showFirstNodeForSplitting()
{
	nodes[0]->setTransparency(true);
	for (UINT i = 0; i < nodes[0]->dockingPorts.size(); ++i)
	{
		if (nodes[0]->dockingPorts[i].docked)
		{
			nodes[0]->dockingPorts[i].helperNode->setVisible(true);
		}
	}
}

//resets the first node to default appearance
void VesselStack::resetFirstNode()
{
	nodes[0]->setTransparency(false);
	for (UINT i = 0; i < nodes[0]->dockingPorts.size(); ++i)
	{
		nodes[0]->dockingPorts[i].helperNode->setVisible(false);
	}
}

std::string VesselStack::toString()
{
    std::string output;
    for (UINT i = 0; i < nodes.size(); ++i)
    {
        
        output += std::to_string(nodes[i]->getUID());
        output += " ";
    }
    return output;
}