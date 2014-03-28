#pragma once

#include <vector>
#include <algorithm>
#include "VesselSceneNode.h"
#include "OrbiterDockingPort.h"

class VesselStack
{
public:
	VesselStack(VesselSceneNode* startingVessel);

private:
	//recursive helper to init the stack
	void createStackHelper(VesselSceneNode* startingVessel, OrbiterDockingPort* fromPort);

	std::vector<VesselSceneNode*> nodes;
};