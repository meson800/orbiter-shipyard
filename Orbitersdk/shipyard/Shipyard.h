#pragma once

#include <irrlicht.h>

#include "VesselSceneNode.h"

using namespace irr;

class Shipyard
{
public:
	Shipyard(IrrlichtDevice * _device) : device(_device) {}
	void loop();

private:
	IrrlichtDevice * device;
};