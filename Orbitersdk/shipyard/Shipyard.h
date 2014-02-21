#pragma once

#include <irrlicht.h>

#include "VesselSceneNode.h"
#include "CSceneNodeAnimatorCameraCustom.h"

#define cameraRotateSpeed = .1;

using namespace irr;

class Shipyard : public IEventReceiver
{
public:
	void setupDevice(IrrlichtDevice * _device);
	void loop();
	bool OnEvent(const SEvent & event);

private:
	core::vector3df returnMouseRelativePos();
	
	IrrlichtDevice * device;
	scene::ICameraSceneNode* camera;
	scene::ISceneNode * selectedNode;
	scene::ISceneCollisionManager* collisionManager;
	scene::ISceneManager* smgr;
	core::vector3df originalMouse3DPos;
	core::vector3df originalNodePosition;
};