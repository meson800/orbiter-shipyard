#pragma once

#include <irrlicht.h>

#include "VesselSceneNode.h"

using namespace irr;

class Shipyard : public IEventReceiver
{
public:
	void setupDevice(IrrlichtDevice * _device) { device = _device; }
	void loop();
	bool OnEvent(const SEvent & event);

private:
	IrrlichtDevice * device;
};