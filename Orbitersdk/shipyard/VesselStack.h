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
private:
	//recursive helper to init the stack
	void createStackHelper(VesselSceneNode* startingVessel, OrbiterDockingPort* fromPort);


	std::vector<VesselSceneNode*> nodes;
};