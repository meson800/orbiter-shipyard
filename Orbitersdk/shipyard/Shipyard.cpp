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
	device = NULL;
}

Shipyard::~Shipyard()
{
	saveToolBoxes();
	Helpers::writeToLog(std::string("\n Terminating StackEditor..."));
}

void Shipyard::setupDevice(IrrlichtDevice * _device, std::string toolboxSet)
{
	device = _device;
	smgr = device->getSceneManager();
	collisionManager = smgr->getSceneCollisionManager();
	guiEnv = device->getGUIEnvironment();
	tbxSet = toolboxSet;

	dataManager.Initialise(device);

	//initialising GUI skin to something nicer and loading a bigger font.
	//well, loading the font, anyways. They messed around with the color identifiers since the last irrlicht version, it'll take a while to set the skin up properly :/
	IGUIFont * font = guiEnv->getFont("StackEditor/deffont.bmp");

	gui::IGUISkin* Skin = guiEnv->getSkin();
	Skin->setFont(font);

	//interface option 1 - somewhat dark, I don't like it much.
/*	scenebgcolor = video::SColor(0, 9, 26, 24);
	Skin->setColor(EGDC_3D_FACE, video::SColor(50, 0, 0, 0));
	Skin->setColor(EGDC_3D_HIGH_LIGHT, video::SColor(50, 0, 0, 0));
	Skin->setColor(EGDC_BUTTON_TEXT, video::SColor(255, 169, 231, 246));
	Skin->setColor(EGDC_HIGH_LIGHT_TEXT, video::SColor(255, 255, 238, 238));
	Skin->setColor(EGDC_HIGH_LIGHT, video::SColor(10, 169, 231, 246));*/
	
	//interface option 2, somewhat inspired by homeworld 2. pretty easy on the eyes if the images are generated with the right background color (regenerate images if image background is black!)
	scenebgcolor = video::SColor(0, 3, 20, 35);
	Skin->setColor(EGDC_3D_FACE, video::SColor(200, 13, 161, 247));
	Skin->setColor(EGDC_3D_HIGH_LIGHT, video::SColor(200, 13, 161, 247));
	Skin->setColor(EGDC_BUTTON_TEXT, video::SColor(255, 255, 255, 255));
	Skin->setColor(EGDC_HIGH_LIGHT_TEXT, video::SColor(255, 255, 222, 185)); 
	Skin->setColor(EGDC_HIGH_LIGHT, video::SColor(200, 0, 33, 70));
	Skin->setColor(EGDC_SCROLLBAR, video::SColor(100, 0, 0, 0));
	Skin->setColor(EGDC_EDITABLE, video::SColor(255, 13, 161, 247));
	Skin->setColor(EGDC_FOCUSED_EDITABLE, video::SColor(255, 13, 161, 247));
	core::dimension2d<u32> dim = device->getVideoDriver()->getScreenSize();

	//initialisng listbox to show available toolboxes
	toolBoxList = guiEnv->addListBox(rect<s32>(0, 0, 100, dim.Height - 130), 0, TOOLBOXLIST, true);
	toolBoxList->setName("Toolboxes");

	bool haderrors = loadToolBoxes();
	if (haderrors)
	//pop a message to tell the user that some of the entries didn't load
	{
		guiEnv->addMessageBox(L"oops...", L"one or more toolbox entries failed to load! \n see StackEditor.log in your Orbiter/StackEditor directory for details.");
	}
	if (toolboxes.size() == 0)
	//adding an empty toolbox in case there are none defined, since you can't even add toolboxes if there is none
	{
		Helpers::writeToLog(std::string("\n WARNING: No toolboxes loaded, initialising default..."));
		toolboxes.push_back(new CGUIToolBox("empty toolbox", rect<s32>(0, dim.Height - 130, dim.Width, dim.Height), guiEnv, NULL));
		guiEnv->getRootGUIElement()->addChild(toolboxes[toolboxes.size() - 1]);
		toolBoxList->addItem(L"empty toolbox");
	}
	activetoolbox = 0;
	toolBoxList->setSelected(activetoolbox);
	switchToolBox();
	
	Helpers::writeToLog(std::string("\n Initialisation complete..."));
}

void Shipyard::centerCamera()
{
//	camera->setTarget(returnOverallBoundingBox().getCenter());
}

void Shipyard::loop()
{
	video::IVideoDriver* driver = device->getVideoDriver();
	
	//add the camera
	camera = new ShipyardCamera(vector3d<f32>(0, 0, 0), 30, smgr->addCameraSceneNode());

/*		camera = smgr->addCameraSceneNode();
	if (camera)
	{
		scene::ISceneNodeAnimator* anm = new scene::CSceneNodeAnimatorCameraCustom(device->getCursorControl());

		camera->addAnimator(anm);
		anm->drop();
	}*/
	
	smgr->setAmbientLight(SColor(150,150,150,150));
	scene::ILightSceneNode *light = smgr->addLightSceneNode(0, core::vector3df(200, 282, 200),
		video::SColorf(0.2f, 0.2f, 0.2f));
	light->setRadius(2000);


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

		Helpers::videoDriverMutex.lock();
		driver->beginScene(true, true, scenebgcolor);
		
		smgr->drawAll();

		guiEnv->drawAll();

		driver->endScene();
		Helpers::videoDriverMutex.unlock();

		//checking toolboxes for vessels to be created
		for (UINT i = 0; i < toolboxes.size(); ++i)
		{
			ToolboxData* toolboxData = toolboxes[i]->checkCreateVessel();
			if (toolboxData != NULL)
			{
				VesselData *createVessel = dataManager.GetGlobalConfig(toolboxData->configFileName, driver);
				if (createVessel != NULL)
				{
					addVessel(createVessel);
				}
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

void Shipyard::addVessel(VesselData* vesseldata)
{
	//add the vessel
	VesselSceneNode* newvessel = new VesselSceneNode(vesseldata, smgr->getRootSceneNode(), smgr, VESSEL_ID);
	vessels.push_back(newvessel);
	//map dockport nodes to map
	for (vector<OrbiterDockingPort>::iterator i = newvessel->dockingPorts.begin(); i != newvessel->dockingPorts.end(); ++i)
	{
		dockportmap.insert(make_pair((*i).portNode, newvessel));
	}

	//if this is the first vessel, center the camera
	if (vessels.size() == 1)
		centerCamera();
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
	
	//EGET_LISTBOX_CHANGED seems to fire unreliably, so we have to check it ourselves
	//also, some events fire before setupShipyard() is called, so we have to make sure that it has already been initialised
	if (device && activetoolbox != toolBoxList->getSelected())
	{
		switchToolBox();
	}

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
			toolboxes[UINT(toolBoxList->getSelected())]->addElement(dataManager.GetGlobalToolboxData(filename, device->getVideoDriver()));
			//reopen file dialog to make multiple selections easier
			guiEnv->addFileOpenDialog(L"Select Config File", true, 0, -1);
			break;
		}
/*		case gui::EGET_LISTBOX_CHANGED:
		{
			switchToolBox();
			break;
		}*/
		case gui::EGET_MESSAGEBOX_OK:
		{
			if (event.GUIEvent.Caller->getID() == TBXNAMEMSG)
			//a name for a new toolbox has been entered, create it
			{
				core::dimension2d<u32> dim = device->getVideoDriver()->getScreenSize();
				IGUIEditBox *toolboxName = (gui::IGUIEditBox*)event.GUIEvent.Caller->getElementFromId(TBXNAMEENTRY, true);
				std::string strName = std::string(stringc(toolboxName->getText()).c_str());

				if (strName != "")
				{
					toolboxes.push_back(new CGUIToolBox(strName, rect<s32>(0, dim.Height - 130, dim.Width, dim.Height), guiEnv, NULL));
					toolBoxList->addItem(toolboxName->getText());
					guiEnv->getRootGUIElement()->addChild(toolboxes[toolboxes.size() - 1]);
					switchToolBox();
				}
			}
			else if (event.GUIEvent.Caller->getID() == TBXDELETE)
			//delete the active toolbox
			{
				CGUIToolBox *doomed = toolboxes[activetoolbox];
				//remove the toolbox all lists
				toolBoxList->removeItem(activetoolbox);
				toolboxes.erase(toolboxes.begin() + activetoolbox);
				//reset the toolbox list and the active toolbox
				activetoolbox = 0;
				toolBoxList->setSelected(activetoolbox);
				switchToolBox();
				//delete the toolbox file from the harddrive and drop it from the GUI
				doomed->deleteToolBoxFromDisk(tbxSet + "/");
				doomed->remove();
			}
			break;
		}
		case gui::EGET_MENU_ITEM_SELECTED:
			if (event.GUIEvent.Caller->getID() == TOOLBOXCONTEXT)
			//toolbox context menu
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
					IGUIWindow *msgBx = guiEnv->addMessageBox(L"toolbox name", L"please enter a name for your toolbox", true, EMBF_OK | EMBF_CANCEL, 0, TBXNAMEMSG);
					msgBx->setMinSize(dimension2du(300, 150));
					guiEnv->addEditBox(L"toolbox name", rect<s32>(20, 100, 280, 130), true, msgBx, TBXNAMEENTRY);
				}
				if (menuItem == 3)
				//pop a confirmation message for deletion
				{
					if (toolBoxList->getItemCount() > 1)
					{
						IGUIWindow *msgBx = guiEnv->addMessageBox(L"confirm or deny", 
							L"this will permanently delete the currently active toolbox. \n Are you sure?", 
							true, EMBF_OK | EMBF_CANCEL, 0, TBXDELETE);
					}
					else
					//there's only one toolbox left. If we delete that, there will be mayhem.
					{
						IGUIWindow *msgBx = guiEnv->addMessageBox(L"I'm afraid I can't do that, Dave...", 
							L"There is only one toolbox left, and Stackeditor needs at least one to function properly! \n \n If you really need to delete this toolbox, create an empty one first.");
					}
				}
			}
			break;
		
		case gui::EGET_FILE_CHOOSE_DIALOG_CANCELLED:
			isOpenDialogOpen = false;
			break;
		case gui::EGET_ELEMENT_HOVERED:
			if (!camera->IsActionInProgress())
			{
				guiEnv->setFocus(event.GUIEvent.Caller);
				cursorOnGui = true;
			}
			break;
		case gui::EGET_ELEMENT_LEFT:
			guiEnv->removeFocus(event.GUIEvent.Caller);
			cursorOnGui = false;
			break;
		}

		break;
	case EET_KEY_INPUT_EVENT:
		//it's a key, store it
		isKeyDown[event.KeyInput.Key] = event.KeyInput.PressedDown;

		//see if it is the "c" key, to center the camera
		if (event.KeyInput.Key == KEY_KEY_C)
			centerCamera();

		//if we are dragging a node, see if it is a rotation
		if (selectedVesselStack != 0 && event.KeyInput.PressedDown)
		{
			bool didWeRotate = false;
			switch (event.KeyInput.Key)
			{
			case KEY_KEY_A:
				selectedVesselStack->rotateStack(core::vector3df(0, -90, 0));
				didWeRotate = true;
				break;
			case KEY_KEY_D:
				selectedVesselStack->rotateStack(core::vector3df(0, 90, 0));
				didWeRotate = true;
				break;
			case KEY_KEY_S:
				selectedVesselStack->rotateStack(core::vector3df(0, 0, -90));
				didWeRotate = true;
				break;
			case KEY_KEY_W:
				selectedVesselStack->rotateStack(core::vector3df(0, 0, 90));
				didWeRotate = true;
				break;
			case KEY_KEY_Q:
				selectedVesselStack->rotateStack(core::vector3df(-90, 0, 0));
				didWeRotate = true;
				break;
			case KEY_KEY_E:
				selectedVesselStack->rotateStack(core::vector3df(90, 0, 0));
				didWeRotate = true;
				break;
			default:
				break;
			}
			if (didWeRotate)
			{
				//reset the move reference, as we screwed up the initial positions
				selectedVesselStack->setMoveReference(returnMouseRelativePos());
			}
		}
		break;
	case EET_MOUSE_INPUT_EVENT:
		//return if cursor is over the GUI, so the GUI gets the event
		if (cursorOnGui)
			return false;
		switch (event.MouseInput.Event)
		{
		case EMIE_RMOUSE_LEFT_UP:
			//tell the camera that the rotation has stopped. it'll probably go and throw up now...
			camera->StopRotation();
			//set the move reference again so the selected stack doesn't teleport into infinity like before
			if (selectedVesselStack != 0)
				selectedVesselStack->setMoveReference(returnMouseRelativePos());
			break;
		case EMIE_RMOUSE_PRESSED_DOWN:
			//make the camera rotate around the current target
			camera->StartRotation();
			break;
		case EMIE_MMOUSE_LEFT_UP:
			//tell the camera to stop translating
			camera->StopTranslation();
			break;
		case EMIE_MMOUSE_PRESSED_DOWN:
			//make the camera pan over the scene
			camera->StartTranslation();
			break;
		case EMIE_LMOUSE_PRESSED_DOWN:
		{
			//if we have a selected node, deselect it
			if (selectedVesselStack != 0)
			{

				//try docking this node
				scene::ISceneNode* selectedNode = collisionManager->getSceneNodeFromScreenCoordinatesBB(
					device->getCursorControl()->getPosition(), DOCKPORT_ID, true);
				if (selectedNode->getID() == DOCKPORT_ID)
				{
					selectedVesselStack->checkForSnapping(dockportmap.find(selectedNode)->second, selectedNode, true);
				}

				//de-show docking ports
				selectedVesselStack->changeDockingPortVisibility(false, false);
				//deshow all the rest of the empty docking ports
				for (unsigned int i = 0; i < vessels.size(); i++)
				{
					if (!selectedVesselStack->isVesselInStack(vessels[i]))
					{
						vessels[i]->changeDockingPortVisibility(false, false);
					}
				}

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
				{
					if (!selectedVesselStack->isVesselInStack(vessels[i]))
					{
						vessels[i]->changeDockingPortVisibility(true, false);
					}
				}
			}

			//return true if we got a vessel stack
			return (selectedVesselStack != 0);
			break;
		}
		case EMIE_MOUSE_MOVED:
			//see if we have a node AND the camera isn't rotating or translating
			if (selectedVesselStack != 0 && !camera->IsActionInProgress())
			{
				//move the stack
				selectedVesselStack->moveStackReferenced(returnMouseRelativePos());

				//try snapping
				scene::ISceneNode* selectedNode = collisionManager->getSceneNodeFromScreenCoordinatesBB(
					device->getCursorControl()->getPosition(), DOCKPORT_ID, true);
				if (selectedNode->getID() == DOCKPORT_ID)
				{
					selectedVesselStack->checkForSnapping(dockportmap.find(selectedNode)->second, selectedNode);
				}
				//selectedVesselStack->checkForSnapping(vessels);
			}
			//update camera position
			camera->UpdatePosition(event.MouseInput.X, event.MouseInput.Y);
			break;
		case EMIE_MOUSE_WHEEL:
			camera->UpdateRadius(event.MouseInput.Wheel);
			break;
		}
		break;
	}
	return false;
}


bool Shipyard::loadToolBoxes()
//loads all tbx files from the Toolbox directory
//returns false if any of the toolbox entries failed to load. The rest will be loaded none the less.
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
		return false;
	}

	bool haderrors = false;
	//searching all tbx files in the directory
	while (searchFiles)
	{
		if (foundFile.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
		{
			//creating the toolbox and adding it to the GUI
			Helpers::writeToLog(std::string("\n Loading " + std::string(foundFile.cFileName) + "..."));

			std::string toolboxName(foundFile.cFileName);
			toolboxes.push_back(new CGUIToolBox(toolboxName.substr(0, toolboxName.size() - 4), rect<s32>(0, dim.Height - 130, dim.Width, dim.Height), guiEnv, NULL));
			guiEnv->getRootGUIElement()->addChild(toolboxes[toolboxes.size() - 1]);

			//adding the toolbox to the toolbox list
			toolBoxList->addItem(stringw(toolboxes[toolboxes.size() - 1]->getName().c_str()).c_str());

			//open the tbx file
			ifstream tbxFile = ifstream(tbxPath + foundFile.cFileName);
			std::string line;
			while (getline(tbxFile, line))
			//loading the toolbox entries
			{
				bool success = toolboxes[toolboxes.size() - 1]->addElement(dataManager.GetGlobalToolboxData(line, device->getVideoDriver()));
				if (!success)
				{
					haderrors = true;
				}
			}
			tbxFile.close();
		}
		searchFiles = FindNextFile(searchFileHndl, &foundFile);
	}
	FindClose(searchFileHndl);

	return !haderrors;			
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
	activetoolbox = toolBoxList->getSelected();
	for (UINT i = 0; i < toolboxes.size(); ++i)
	{
		if (i == activetoolbox)
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