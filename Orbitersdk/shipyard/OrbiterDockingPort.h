#pragma once

#include <irrlicht.h>

using namespace irr;

struct OrbiterDockingPort
{
	OrbiterDockingPort(core::vector3d<f32> pos, core::vector3d<f32> appDir, core::vector3d<f32> refDir)
	: position(pos), approachDirection(appDir), referenceDirection(refDir) {}
	core::vector3d<f32> position;
	core::vector3d<f32> approachDirection;
	core::vector3d<f32> referenceDirection;
};