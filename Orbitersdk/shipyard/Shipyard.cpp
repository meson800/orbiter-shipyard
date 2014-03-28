#include "Shipyard.h"
#include "GuiIdentifiers.h"
#include "windows.h"

Shipyard::Shipyard()
{
	for (u32 i = 0; i<KEY_KEY_CODES_COUNT; ++i)
		isKeyDown[i] = false;
	
	isOpenDialogOpen = false;
	selectedVesselStack = 0;
	cursorOnGui = false;
}

Shipyard::~Shipyard()
{
	saveToolBoxes();
	Helpers::writeToLog(std::string("\n Terminating StackEditor..."), true);
}

void Shipyard::setupDevice(IrrlichtDevice * _device, std::string toolboxSet)
{
	device = _device;
	smgr = device->getSceneManager();
	collisionManager = smgr->getSceneCollisionManager();
	guiEnv = device->getGUIEnvironment();
	tbxSet = toolboxSet;

	//initialising GUI skin to something nicer and loading a bigger font.
	//well, loading the font, anyways. They messed around with the color identifiers since the last irrlicht version, it'll take a while to set the skin up properly :/
	IGUIFont * font = guiEnv->getFont("StackEditor/deffont.bmp");

	gui::IGUISkin* Skin = guiEnv->getSkin();
	Skin->setFont(font);
/*	Skin->setColor(EGDC_3D_FACE, video::SColor(255, 43, 44, 131));
	Skin->setColor(EGDC_HIGH_LIGHT, video::SColor(255, 207, 183, 52));
	Skin->setColor(EGDC_HIGH_LIGHT_TEXT, video::SColor(255, 4, 5, 76));
	Skin->setColor(EGDC_BUTTON_TEXT, video::SColor(255, 255, 255, 255));
	Skin->setColor(EGDC_3D_SHADOW, video::SColor(255, 43, 44, 131));
	Skin->setColor(EGDC_3D_HIGH_LIGHT, video::SColor(255, 43, 44, 131));
	Skin->setColor(EGDC_3D_LIGHT, video::SColor(255, 43, 44, 131));
	Skin->setColor(EGDC_TOOLTIP, video::SColor(255, 4, 5, 76));
	Skin->setColor(EGDC_TOOLTIP_BACKGROUND, video::SColor(255, 207, 183, 52));
	Skin->setColor(EGDC_SCROLLBAR, video::SColor(179, 207, 183, 52));
	//Skin->setColor(EGDC_WINDOW, video::SColor(255, 4, 5, 76));
	Skin->setColor(EGDC_EDITABLE, video::SColor(255, 207, 183, 52));
	Skin->setColor(EGDC_FOCUSED_EDITABLE, video::SColor(255, 43, 44, 131));*/

	core::dimension2d<u32> dim = device->getVideoDriver()->getScreenSize();

	//initialisng listbox to show available toolboxes
	toolBoxList = guiEnv->addListBox(rect<s32>(0, 0, 100, dim.Height - 210), 0, TOOLBOXLIST, true);
	toolBoxList->setName("Toolboxes");

	loadToolBoxes();

	if (toolboxes.size() == 0)
	//adding an empty toolbox in case there are none defined, since you can't even add toolboxes if there is none
	{
		Helpers::writeToLog(std::string("\n WARNING: No toolboxes loaded, initialising default..."));
		toolboxes.push_back(new CGUIToolBox("empty toolbox", rect<s32>(0, dim.Height - 210, dim.Width, dim.Height), guiEnv, NULL));
		guiEnv->getRootGUIElement()->addChild(toolboxes[toolboxes.size() - 1]);
		toolBoxList->addItem(L"empty toolbox");
	}
	toolBoxList->setSelected(0);
	switchToolBox();
	
	Helpers::writeToLog(std::string("\n Initialisation complete..."));
}

void Shipyard::centerCamera()
{
	camera->setTarget(returnOverallBoundingBox().getCenter());
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
	//vessels.push_back(new VesselSceneNode("C:\\Other Stuff\\Orbiter\\shipyard\\Config\\Vessels\\ProjectAlpha_ISS.cfg", 
	//	smgr->getRootSceneNode(), smgr, 72));

	//start the loop
	while (device->run())
	{
		//see if we are pressing control o- so we can open a open file dialog
		if (isKeyDown[KEY_LCONTROL] || isKeyDown[KEY_RCONTROL] || isKeyDown[KEY_CONTROL])
		{
//			device->postEventFromUser(EMIE_MMOUSE_PRESSED_DOWN);
		}

		driver->beginScene(true, true, video::SColor(0, 100, 100, 100));

		smgr->drawAll();

		guiEnv->drawAll();

		driver->endScene();

		//checking toolboxes for vessels to be created
		for (UINT i = 0; i < toolboxes.size(); ++i)
		{
			VesselData *createVessel = toolboxes[i]->checkCreateVessel();
			if (createVessel != NULL)
			{
				vessels.push_back(new VesselSceneNode(createVessel, smgr->getRootSceneNode(), smgr, VESSEL_ID));
				//create a selected stack
//				selectedVesselStack = new VesselStack(vessels[vessels.size() - 1]);
			}

		}
		
	}
	//drop all vessels
	for (unsigned int i = 0; i < vessels.size(); i++)
	{
		vessels[0]->drop();
		vessels.erase(vessels.begin());
	}
}

core::vector3df Shipyard::returnMouseRelativePos()
{
	core::vector3df mouse3DPos;
	if (selectedVesselStack != 0 && selectedVesselStack->numVessels() != 0)
	{
		//update the absolute position
		selectedVesselStack->getVessel(0)->updateAbsolutePosition();
		//the plane point is from the center of the node, with the normal to the camera
		core::plane3df plane = core::plane3df(selectedVesselStack->getVessel(0)->getAbsolutePosition(),
			camera->getTarget() - camera->getPosition());

		//now do a ray from the current mouse position
		core::line3df ray = collisionManager->getRayFromScreenCoordinates(device->getCursorControl()->getPosition());
		//if they intersect, set it equal to mouse3DPos
		plane.getIntersectionWithLine(ray.start, ray.getVector(), mouse3DPos);
	}
	return mouse3DPos;
}

core::aabbox3d<f32> Shipyard::returnOverallBoundingBox()
{
	core::aabbox3d<f32> result;
	for (unsigned int i = 0; i < vessels.size(); i++)
		result.addInternalBox(vessels[i]->getTransformedBoundingBox());
	return result;
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
			
			std::string fullfilename = std::string(uniString.begin(), uniString.end());
			std::string filename = fullfilename.substr(Helpers::workingDirectory.length() + 16);
			//create a new toolbox entry
			toolboxes[UINT(toolBoxList->getSelected())]->addElement(dataManager.GetGlobalConfig(filename, device->getVideoDriver()));
			//reopen file dialog to make multiple selections easier
			guiEnv->addFileOpenDialog(L"Select Config File", true, 0, -1);
			break;
		}
		case gui::EGET_LISTBOX_CHANGED:
		{
			switchToolBox();
			break;
		}
		case gui::EGET_MESSAGEBOX_OK:
		{
			if (event.GUIEvent.Caller->getID() == TBXNAMEMSG)
			{
				core::dimension2d<u32> dim = device->getVideoDriver()->getScreenSize();
				IGUIEditBox *toolboxName = (gui::IGUIEditBox*)event.GUIEvent.Caller->getElementFromId(TBXNAMEENTRY, true);
				std::string strName = std::string(stringc(toolboxName->getText()).c_str());

				if (strName != "")
				{
					toolboxes.push_back(new CGUIToolBox(strName, rect<s32>(0, dim.Height - 210, dim.Width, dim.Height), guiEnv, NULL));
					toolBoxList->addItem(toolboxName->getText());
					guiEnv->getRootGUIElement()->addChild(toolboxes[toolboxes.size() - 1]);
					switchToolBox();
				}
			}
			break;
		}
		case gui::EGET_MENU_ITEM_SELECTED:
			if (event.GUIEvent.Caller->getID() == TOOLBOXCONTEXT)
			{
				s32 menuItem = ((gui::IGUIContextMenu*)event.GUIEvent.Caller)->getSelectedItem();
				if (menuItem == 0)
				//open file dialog to add new element
				{
					IGUIFileOpenDialog *dlg = guiEnv->addFileOpenDialog(L"Select Config File", true, 0, -1, false, "config\\vessels");
				}
				if (menuItem == 1)
				{
					toolboxes[UINT(toolBoxList->getSelected())]->removeCurrentElement();
				}
				if (menuItem == 2)
				//spawn a naming dialog
				{
//					selectedNode = 0;
					IGUIWindow *msgBx = guiEnv->addMessageBox(L"toolbox name", L"please enter a name for your toolbox", true, EMBF_OK | EMBF_CANCEL, 0, TBXNAMEMSG);
					msgBx->setMinSize(dimension2du(300, 150));
					guiEnv->addEditBox(L"toolbox name", rect<s32>(20, 100, 280, 130), true, msgBx, TBXNAMEENTRY);
				}
			}
			break;
		
		case gui::EGET_FILE_CHOOSE_DIALOG_CANCELLED:
			isOpenDialogOpen = false;
			break;
		case gui::EGET_ELEMENT_HOVERED:
			cursorOnGui = true;
			break;
		case gui::EGET_ELEMENT_LEFT:
			cursorOnGui = false;
			break;
		}

		break;
	case EET_KEY_INPUT_EVENT:
		//it's a key, store it
		isKeyDown[event.KeyInput.Key] = event.KeyInput.PressedDown;
		//if we are dragging a node, see if it is a rotation
		if (selectedVesselStack != 0 && event.KeyInput.PressedDown)
		{
			switch (event.KeyInput.Key)
			{
			case KEY_KEY_A:
				selectedVesselStack->rotateStack(core::vector3df(0, -90, 0));
				break;
			case KEY_KEY_D:
				selectedVesselStack->rotateStack(core::vector3df(0, 90, 0));
				break;
			case KEY_KEY_S:
				selectedVesselStack->rotateStack(core::vector3df(0, 0, -90));
				break;
			case KEY_KEY_W:
				selectedVesselStack->rotateStack(core::vector3df(0, 0, 90));
				break;
			case KEY_KEY_Q:
				selectedVesselStack->rotateStack(core::vector3df(-90, 0, 0));
				break;
			case KEY_KEY_E:
				selectedVesselStack->rotateStack(core::vector3df(90, 0, 0));
				break;
			default:
				break;
			}
		}
		break;
	case EET_MOUSE_INPUT_EVENT:
		//return if cursor is over the GUI, so the GUI gets the event
		if (cursorOnGui)
			return false;
		switch (event.MouseInput.Event)
		{
		case EMIE_LMOUSE_PRESSED_DOWN:
		{
			//if we have a selected node, deselect it
			if (selectedVesselStack != 0)
			{
				//try docking this node
				selectedVesselStack->checkForSnapping(vessels, true);

				//de-show docking ports
				selectedVesselStack->changeDockingPortVisibility(false, false);
				//deshow all the rest of the empty docking ports
				for (unsigned int i = 0; i < vessels.size(); i++)
					vessels[i]->changeDockingPortVisibility(false, false);

				delete selectedVesselStack;
				selectedVesselStack = 0;
				return true;
			}
			//try to select a node
			selectedVesselStack = 0;
			scene::ISceneNode* selectedNode = collisionManager->getSceneNodeFromScreenCoordinatesBB(
				device->getCursorControl()->getPosition(), VESSEL_ID, true);
			if (selectedNode->getID() == VESSEL_ID)
			{
				//it's a vessel!  Create the stack
				selectedVesselStack = new VesselStack((VesselSceneNode*)selectedNode);
				selectedNode = 0;
			}
			if (selectedVesselStack != 0)
			{
				//setup the move reference
				selectedVesselStack->setMoveReference(returnMouseRelativePos());

				//show empty docking ports
				selectedVesselStack->changeDockingPortVisibility(true, false);
				//show all the rest of the empty docking ports
				for (unsigned int i = 0; i < vessels.size(); i++)
					vessels[i]->changeDockingPortVisibility(true, false);
			}

			//return true if we got a vessel stack
			return (selectedVesselStack != 0);
			break;
		}
		case EMIE_MOUSE_MOVED:
			//see if we have a node
			if (selectedVesselStack != 0)
			{
				//move the stack
				selectedVesselStack->moveStackReferenced(returnMouseRelativePos());

				//try snapping
				selectedVesselStack->checkForSnapping(vessels);
			}
			break;
		}
		break;
	}
	return false;
}


bool Shipyard::loadToolBoxes()
//loads all tbx files from the Toolbox directory
{
	core::dimension2d<u32> dim = device->getVideoDriver()->getScreenSize();
	std::string tbxPath = std::string(Helpers::workingDirectory + "/StackEditor/Toolboxes/" + tbxSet + "/");
	Helpers::writeToLog(std::string("\n Loading toolbox set: ") + tbxSet);
	std::string searchPath = std::string(tbxPath + "*.tbx");
	HANDLE searchFileHndl;
	WIN32_FIND_DATA foundFile;
	//initialising the first character to check if FindFirstFile was succesfull
	foundFile.cFileName[0] = 0;
	
	vector<std::string> entriesToLoad;

	bool searchFiles = true;
	
	searchFileHndl = FindFirstFile(searchPath.data(), &foundFile);
	if (foundFile.cFileName[0] == 0)
	{
		Helpers::writeToLog(std::string("\n ERROR: could not open directory: /StackEditor/Toolboxes/" + tbxSet + " or no files in directory"));
		searchFiles = false;
	}

	//searching all tbx files in the directory
	while (searchFiles)
	{
		if (foundFile.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
		{
			//creating the toolbox and adding it to the GUI
			Helpers::writeToLog(std::string("\n Loading " + std::string(foundFile.cFileName) + "..."));

			std::string toolboxName(foundFile.cFileName);
			toolboxes.push_back(new CGUIToolBox(toolboxName.substr(0, toolboxName.size() - 4), rect<s32>(0, dim.Height - 210, dim.Width, dim.Height), guiEnv, NULL));
			guiEnv->getRootGUIElement()->addChild(toolboxes[toolboxes.size() - 1]);

			//adding the toolbox to the toolbox list
			toolBoxList->addItem(stringw(toolboxes[toolboxes.size() - 1]->getName().c_str()).c_str());

			//open the tbx file
			ifstream tbxFile = ifstream(tbxPath + foundFile.cFileName);
			std::string line;
			while (getline(tbxFile, line))
			//loading the toolbox entries
			{
				toolboxes[toolboxes.size() - 1]->addElement(dataManager.GetGlobalConfig(line, device->getVideoDriver()));
			}
			tbxFile.close();
		}
		searchFiles = FindNextFile(searchFileHndl, &foundFile);
	}
	FindClose(searchFileHndl);

	return false;
}


void Shipyard::saveToolBoxes()
{
	
	for (UINT i = 0; i < toolboxes.size(); ++i)
	{
		toolboxes[i]->saveToolBox(std::string(tbxSet + "/"));
	}
}


void Shipyard::switchToolBox()
{
	int selBox = toolBoxList->getSelected();
	for (UINT i = 0; i < toolboxes.size(); ++i)
	{
		if (i == selBox)
		{
			toolboxes[i]->setVisible(true);
			toolboxes[i]->bringToFront(toolboxes[i]);
		}
		else
		{
			toolboxes[i]->setVisible(false);
		}
	}

}