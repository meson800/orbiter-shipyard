#include "Shipyard.h"

void Shipyard::setupDevice(IrrlichtDevice * _device)
{
	device = _device;
	smgr = device->getSceneManager();
	collisionManager = smgr->getSceneCollisionManager();
}

void Shipyard::loop()
{
	video::IVideoDriver* driver = device->getVideoDriver();
	
	//add the camera
	camera = smgr->addCameraSceneNodeMaya();
	camera = smgr->addCameraSceneNode();
	if (camera)
	{
		scene::ISceneNodeAnimator* anm = new scene::CSceneNodeAnimatorCameraCustom(device->getCursorControl());

		camera->addAnimator(anm);
		anm->drop();
	}

	smgr->setAmbientLight(SColor(150,150,150,150));

	//register our VesselSceneNode - just staticly at the moment, but will do it later
	VesselSceneNode* vesselNode = new VesselSceneNode("C:\\Other Stuff\\Orbiter\\shipyard\\Config\\Vessels\\ProjectAlpha_ISS.cfg", 
		smgr->getRootSceneNode(), smgr, 72);
	vesselNode->setupDockingPortNodes();
	vesselNode->drop();

	//start the loop
	while (device->run())
	{
		driver->beginScene(true, true, video::SColor(0, 100, 100, 100));

		smgr->drawAll();

		driver->endScene();
	}
}

core::vector3df Shipyard::returnMouseRelativePos()
{
	core::vector3df mouse3DPos;
	if (selectedNode != 0)
	{
		//update the absolute position
		selectedNode->updateAbsolutePosition();
		//the plane point is from the center of the node, with the normal to the camera
		core::plane3df plane = core::plane3df(selectedNode->getAbsolutePosition(),
			camera->getTarget() - camera->getPosition());

		//now do a ray from the current mouse position
		core::line3df ray = collisionManager->getRayFromScreenCoordinates(device->getCursorControl()->getPosition());
		//if they intersect, set it equal to mouse3DPos
		plane.getIntersectionWithLine(ray.start, ray.getVector(), mouse3DPos);
	}
	return mouse3DPos;
}

bool Shipyard::OnEvent(const SEvent& event)
{
	switch (event.EventType)
	{
	case EET_MOUSE_INPUT_EVENT:
		switch (event.MouseInput.Event)
		{
		case EMIE_LMOUSE_PRESSED_DOWN:
			LMouseDown = true;
			//try to select a node
			selectedNode = collisionManager->getSceneNodeFromScreenCoordinatesBB(
				device->getCursorControl()->getPosition(), 72, true);
			//set mouse position
			oldMouse3DPos = returnMouseRelativePos();
			//return true if we got a node
			return (selectedNode != 0);
			break;

		case EMIE_LMOUSE_LEFT_UP:
			LMouseDown = false;
			return false;
			break;

		case EMIE_MOUSE_MOVED:
			//return if we aren't dragging
			if (LMouseDown == false)
				return false;
			//see if we have a node
			if (selectedNode != 0)
			{
				//calculate new mouse pos
				core::vector3df mouse3DPos = returnMouseRelativePos();
				//move the node by the difference of the two
				selectedNode->setPosition(selectedNode->getPosition() + (mouse3DPos - oldMouse3DPos));
				//update old pos
				oldMouse3DPos = mouse3DPos;
			}
			break;
		}
	}
	return false;
}