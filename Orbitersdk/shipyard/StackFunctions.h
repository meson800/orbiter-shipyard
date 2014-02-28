#pragma once

#include <vector>
#include <algorithm>
#include "VesselSceneNode.h"
#include "OrbiterDockingPort.h"

typedef std::vector<VesselSceneNode*> vesselStack;

class StackFunctions
{
public:
	static void createStack(VesselSceneNode* startingVessel, vesselStack& stack);

private:
	static void createStackHelper(VesselSceneNode* startingVessel, OrbiterDockingPort* fromPort, vesselStack& stack);
};