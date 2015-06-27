#pragma once

#include <vector>
#include <irrlicht.h>
#include <map>

#include "VesselStack.h"
#include "OrbiterDockingPort.h"

class VesselStackOperations
{
public:
	static void splitStack(OrbiterDockingPort* sourcePort);
	static std::vector<VesselSceneNode*> copyStack(VesselStack* stack, scene::ISceneManager* smgr);
};