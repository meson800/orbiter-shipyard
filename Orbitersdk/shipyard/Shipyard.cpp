#include "Shipyard.h"

Shipyard::Shipyard()
{
	for (u32 i = 0; i<KEY_KEY_CODES_COUNT; ++i)
		isKeyDown[i] = false;
	isOpenDialogOpen = false;
}

void Shipyard::setupDevice(IrrlichtDevice * _device)
{
	device = _device;
	smgr = device->getSceneManager();
	collisionManager = smgr->getSceneCollisionManager();
	guiEnv = device->getGUIEnvironment();
}

void Shipyard::loop()
{
	video::IVideoDriver* driver = device->getVideoDriver();
	
	//add the camera
	camera = smgr->addCameraSceneNode();
	if (camera)
	{
		scene::ISceneNodeAnimator* anm = new scene::CSceneNodeAnimatorCameraCustom(device->getCursorControl());

		camera->addAnimator(anm);
		anm->drop();
	}

	smgr->setAmbientLight(SColor(150,150,150,150));

	//register our VesselSceneNode - just staticly at the moment, but will do it later
	vessels.push_back(new VesselSceneNode("C:\\Other Stuff\\Orbiter\\shipyard\\Config\\Vessels\\ProjectAlpha_ISS.cfg", 
		smgr->getRootSceneNode(), smgr, 72));

	//start the loop
	while (device->run())
	{
		//see if we are pressing control o- so we can open a open file dialog
		if ((isKeyDown[KEY_LCONTROL] || isKeyDown[KEY_RCONTROL] || isKeyDown[KEY_CONTROL])
			&& isKeyDown[KEY_KEY_O] && !isOpenDialogOpen)
		{
			guiEnv->addFileOpenDialog(L"Select Config File", true, 0, -1, true);
			isOpenDialogOpen = true;
		}

		driver->beginScene(true, true, video::SColor(0, 100, 100, 100));

		smgr->drawAll();

		guiEnv->drawAll();

		driver->endScene();
	}
	//drop all vessels
	for (int i = 0; i < vessels.size(); i++)
	{
		vessels[0]->drop();
		vessels.erase(vessels.begin());
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
	case EET_GUI_EVENT:
		switch (event.GUIEvent.EventType)
		{
		case gui::EGET_FILE_SELECTED:
		{
			(gui::IGUIFileOpenDialog*)event.GUIEvent.Caller;
			std::wstring uniString = std::wstring(((gui::IGUIFileOpenDialog*)event.GUIEvent.Caller)->getFileName());
			std::string filename = std::string(uniString.begin(), uniString.end());
			//create a new vessel
			vessels.push_back(new VesselSceneNode(filename, smgr->getRootSceneNode(), smgr, 72));
			isOpenDialogOpen = false;
			break;
		}
		case gui::EGET_FILE_CHOOSE_DIALOG_CANCELLED:
			isOpenDialogOpen = false;
			break;
		}
		break;
	case EET_KEY_INPUT_EVENT:
		//it's a key, store it
		isKeyDown[event.KeyInput.Key] = event.KeyInput.PressedDown;
		//if we are dragging a node, see if it is a rotation
		if (selectedNode != 0 && event.KeyInput.PressedDown)
		{
			core::vector3df currentRotation = selectedNode->getRotation();
			switch (event.KeyInput.Key)
			{
			case KEY_KEY_A:
				currentRotation.Y -= 90;
				break;
			case KEY_KEY_D:
				currentRotation.Y += 90;
				break;
			case KEY_KEY_S:
				currentRotation.Z -= 90;
				break;
			case KEY_KEY_W:
				currentRotation.Z += 90;
				break;
			case KEY_KEY_Q:
				currentRotation.X -= 90;
				break;
			case KEY_KEY_E:
				currentRotation.X += 90;
				break;
			default:
				break;
			}
			//set rotation
			selectedNode->setRotation(currentRotation);
		}
		break;
	case EET_MOUSE_INPUT_EVENT:
		switch (event.MouseInput.Event)
		{
		case EMIE_LMOUSE_PRESSED_DOWN:
			//return if we have the dialog open
			if (isOpenDialogOpen)
				return false;
			//if we have a selected node, deselect it
			if (selectedNode != 0)
			{
				//de-show docking ports
				((VesselSceneNode*)selectedNode)->changeDockingPortVisibility(false, false);
				selectedNode = 0;
				return true;
			}
			//try to select a node
			selectedNode = 0;
			selectedNode = collisionManager->getSceneNodeFromScreenCoordinatesBB(
				device->getCursorControl()->getPosition(), 72, true);
			if (selectedNode != 0)
			{
				//set mouse position
				originalMouse3DPos = returnMouseRelativePos();
				//set old node location
				originalNodePosition = selectedNode->getPosition();

				//show docking ports
				((VesselSceneNode*)selectedNode)->changeDockingPortVisibility(true, true);
			}

			//return true if we got a node
			return (selectedNode != 0);
			break;

		case EMIE_MOUSE_MOVED:
			//see if we have a node
			if (selectedNode != 0)
			{
				//calculate new mouse pos
				core::vector3df mouse3DPos = returnMouseRelativePos();
				//move the node by the difference of the two
				selectedNode->setPosition(originalNodePosition + (mouse3DPos - originalMouse3DPos));
			}
			break;
		}
		break;
	}
	return false;
}