#pragma once
#include <irrlicht.h>

class VesselSceneNode;

using namespace irr;

struct DockingIdentifier
{
    unsigned int vesselUID, portID;
};

struct DockingPortStatus
{
    bool docked;
    DockingIdentifier dockedTo;
};

struct OrbiterDockingPort
{
	OrbiterDockingPort(core::vector3d<f32> pos, core::vector3d<f32> appDir, core::vector3d<f32> refDir)
	: position(pos), approachDirection(appDir), referenceDirection(refDir) {}
	VesselSceneNode* parent;
    DockingIdentifier dockedTo;

    DockingPortStatus returnStatus();

    unsigned int portID;
	scene::IMeshSceneNode* portNode;
	scene::IMeshSceneNode* helperNode;
	int index;
	bool docked;
	core::vector3d<f32> position;
	core::vector3d<f32> approachDirection;
	core::vector3d<f32> referenceDirection;
};