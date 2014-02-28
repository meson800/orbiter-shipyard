#pragma once

#include <irrlicht.h>

class VesselSceneNode;

using namespace irr;

struct OrbiterDockingPort
{
	OrbiterDockingPort(core::vector3d<f32> pos, core::vector3d<f32> appDir, core::vector3d<f32> refDir)
	: position(pos), approachDirection(appDir), referenceDirection(refDir) {}
	VesselSceneNode* parent;
	OrbiterDockingPort* dockedTo;
	scene::ISceneNode* portNode;
	bool docked;
	core::vector3d<f32> position;
	core::vector3d<f32> approachDirection;
	core::vector3d<f32> referenceDirection;
};