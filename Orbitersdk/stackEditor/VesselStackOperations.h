//Copyright (c) 2015 Christopher Johnstone(meson800) and Benedict Haefeli(jedidia)
//The MIT License - See ../../LICENSE for more info
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
	static void deleteStack(VesselStack* stack);
};
