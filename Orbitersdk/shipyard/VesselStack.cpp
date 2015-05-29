#include "VesselStack.h"

VesselStack::VesselStack(VesselSceneNode* startingVessel)
{
	//recurse through with the helper, start with a null pointer, so any docked port works
	createStackHelper(startingVessel, 0);
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
	//apply this rotation to each of the parent nodes
	core::quaternion x, y, z, finalRot;
	x.fromAngleAxis(relativeRot.X * core::DEGTORAD, core::vector3df(1, 0, 0));
	y.fromAngleAxis(relativeRot.Y * core::DEGTORAD, core::vector3df(0, 1, 0));
	z.fromAngleAxis(relativeRot.Z * core::DEGTORAD, core::vector3df(0, 0, 1));
	finalRot = x * y * z;

	//now use the force, erm, quaternoins to rotate each node in the stack to avoid gimbal lock
	for (unsigned int i = 0; i < nodes.size(); i++)
	{
		core::quaternion thisNodeRotation = core::quaternion(nodes[i]->getRotation() * core::DEGTORAD);
		//rotate this sucker
		thisNodeRotation = thisNodeRotation * finalRot;
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
		core::vector3df rotatedPos = finalRot * relativePos;
		nodes[i]->setPosition(nodes[i]->getPosition() + (rotatedPos - relativePos));
	}
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
	//set reference to a null position, and just run the normal referenced move
	//NOTE: this destroys the reference, don't use this in the middle of using moveStackReferenced
	setMoveReference(core::vector3df());
	moveStackReferenced(movePos);
}

void VesselStack::moveStackReferenced(core::vector3df movePos)
{
	//We have to do this system from the reference point because of the way snapping is handled
	//If this just added a relative position to each of the nodes, a small mouse movement would never
	//be able to move the node out of the snapping radius, resulting in permenant snapping.
	//This way always moves the node to a position relative to the initial position, making snapping work

	//check to make sure that we have the same amount of positions in previousPositions than we have nodes
	if (nodes.size() != previousPositions.size())
	{
		Helpers::writeToLog(std::string("Tried to move a vessel stack without setting up previousPositions\n"));
		return;
	}

	//loop over, setting relative position
	for (unsigned int i = 0; i < nodes.size(); i++)
	{
		nodes[i]->setPosition(previousPositions[i] + (movePos - moveReference));
	}

}

void VesselStack::checkForSnapping(VesselSceneNode* vessel, ISceneNode* dockportnode, bool dock)
{
	//loop over each node
/*	for (unsigned int nodeNum = 0; nodeNum < nodes.size(); nodeNum++)
	{
		//loop over empty docking ports on this node
		for (unsigned int i = 0; i < nodes[nodeNum]->dockingPorts.size(); i++)
		{
			if (nodes[nodeNum]->dockingPorts[i].docked == false)
			{
				//now, loop over the OTHER vessels (not equal to this node)
				//and see if it is close to another port's empty docking ports
				for (unsigned int j = 0; j < vessels.size(); j++)
				{
					//make sure this vessel isn't in our stack
					if (std::find(nodes.begin(), nodes.end(), vessels[j]) == nodes.end())
					{
						//check it's docking ports
						for (unsigned int k = 0; k < vessels[j]->dockingPorts.size(); k++)
						{
							//check if it is not docked, and they are within a certain distance
							if (!vessels[j]->dockingPorts[k].docked &&
								((nodes[nodeNum]->getAbsolutePosition() + nodes[nodeNum]->returnRotatedVector(nodes[nodeNum]->dockingPorts[i].position)) -
								(vessels[j]->getAbsolutePosition() + vessels[j]->returnRotatedVector(vessels[j]->dockingPorts[k].position))
								).getLengthSQ() < 16)
							{
								//snap it if dock=false, dock if dock=true
								if (!dock)
									snapStack(nodeNum, i, vessels[j]->dockingPorts[k]);
									//nodes[nodeNum]->snap(nodes[nodeNum]->dockingPorts[i], vessels[j]->dockingPorts[k]);
								else
									nodes[nodeNum]->dock(nodes[nodeNum]->dockingPorts[i], vessels[j]->dockingPorts[k]);
							}

						}
					}
				}
			}
		}
	}*/

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
				float dist = dockportnode->getAbsolutePosition().getDistanceFrom(v->dockingPorts[j].helperNode->getAbsolutePosition());
				if (dist <= closestdist)
					//this one's closer, mark it as the closest so far
				{
					closestvessel = i;
					closestport = j;
				}
			}
		}
	}
	
	if (closestvessel != -1 && closestport != -1)
	{
		//we have a valid vessel in the stack and a valid dockport, let's snap
		snapStack(closestvessel, closestport, vessel->dockingPorts[targetdock]);
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
void VesselStack::snapStack(int srcvesselidx, int srcdockportidx, OrbiterDockingPort& tgtport)
{
	VesselSceneNode *srcvessel = nodes[srcvesselidx];
	//snap the corpus delicti
	srcvessel->snap(srcvessel->dockingPorts[srcdockportidx], tgtport);

	vector<VesselSceneNode*> snapped_in_last_pass;							//will remember the vessels processed in the last pass
	snapped_in_last_pass.push_back(srcvessel);								//this is where we start from
	vector<VesselSceneNode*> hasSnapped = snapped_in_last_pass;				//will contain all vessels that have already snapped
	hasSnapped.push_back(tgtport.parent);									//need that in here too


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
				if (j.docked && std::find(hasSnapped.begin(), hasSnapped.end(), j.dockedTo->parent) == hasSnapped.end())
				//if the dockport has a connection and the connected vessel hasn't snapped already
				{
					j.dockedTo->parent->snap(*j.dockedTo, j);
					snapped_in_this_pass.push_back(j.dockedTo->parent);
					hasSnapped.push_back(j.dockedTo->parent);
				}
			}
		}
		snapped_in_last_pass = snapped_in_this_pass;
	}
}


//recursive function to init a vessel stack
void VesselStack::createStackHelper(VesselSceneNode* startingVessel, OrbiterDockingPort* fromPort)
{
	//recurse through the docking ports, through docked vessels
	//if the nodes doesn't contain this vessel yet, add it
	if (std::find(nodes.begin(), nodes.end(), startingVessel) == nodes.end())
	{
		nodes.push_back(startingVessel);
		for (UINT i = 0; i < startingVessel->dockingPorts.size(); ++i)
		{
			//store all dockingport nodes in the stack. It could occur that the stack tries to dock with itself otherwise
			dockportnodes.push_back(startingVessel->dockingPorts[i].portNode);
		}
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


bool VesselStack::isVesselInStack(VesselSceneNode* vessel)
{
	return std::find(nodes.begin(), nodes.end(), vessel) != nodes.end();
}