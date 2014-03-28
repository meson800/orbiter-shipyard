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
private:
	//recursive helper to init the stack
	void createStackHelper(VesselSceneNode* startingVessel, OrbiterDockingPort* fromPort);

	std::vector<core::vector3df> previousPositions;
	core::vector3df moveReference;
	std::vector<VesselSceneNode*> nodes;
};