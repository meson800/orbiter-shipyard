#pragma once

#include <vector>
#include <algorithm>
#include "VesselSceneNode.h"
#include "OrbiterDockingPort.h"

class VesselStack
{
public:
	VesselStack(VesselSceneNode* startingVessel);
	void rotateStack(core::vector3df relativeRot);
	void setMoveReference(core::vector3df refPos);
	void moveStackReferenced(core::vector3df movePos);
	void moveStackRelative(core::vector3df movePos);
	void checkForSnapping(std::vector<VesselSceneNode*>& vessels, bool dock=false);
	void changeDockingPortVisibility(bool showEmpty, bool showDocked);
	int numVessels();
	VesselSceneNode* getVessel(int index);
private:
	//recursive helper to init the stack
	void createStackHelper(VesselSceneNode* startingVessel, OrbiterDockingPort* fromPort);

	std::vector<core::vector3df> previousPositions;
	core::vector3df moveReference;
	std::vector<VesselSceneNode*> nodes;
};