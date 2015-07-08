#include "Shipyard.h"
#include "GuiIdentifiers.h"
#include "windows.h"


Shipyard::Shipyard(ExportData *exportdata)
{
	for (u32 i = 0; i<KEY_KEY_CODES_COUNT; ++i)
		isKeyDown[i] = false;
	
	isOpenDialogOpen = false;
	selectedVesselStack = 0;
	cursorOnGui = false;
	dialogOpen = false;
	device = NULL;
	lastSpawnedVessel = NULL;
	session = "unnamed";
	areSplittingStack = false;
	_exportdata = exportdata;

    Helpers::setVesselMap(&uidVesselMap);
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

	
	//skin option 2, somewhat inspired by homeworld 2. pretty easy on the eyes if the images are generated with the right background color (regenerate images if image background is black!)
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
	toolBoxList = guiEnv->addListBox(rect<s32>(0, 0, 120, dim.Height - 190), 0, TOOLBOXLIST, true);
	toolBoxList->setName("Toolboxes");
	//save and load buttons
	guiEnv->addButton(rect<s32>(0, dim.Height - 190, 120, dim.Height - 170), 0, SAVESESSION, L"save");
	guiEnv->addButton(rect<s32>(0, dim.Height - 170, 120, dim.Height - 150), 0, SAVESESSIONAS, L"save as");
	guiEnv->addButton(rect<s32>(0, dim.Height - 150, 120, dim.Height - 130), 0, LOADSESSION, L"load");

	bool hadnoerrors = loadToolBoxes();
	if (!hadnoerrors)
	//pop a message to tell the user that some of the entries didn't load
	{
		guiEnv->addMessageBox(L"He's dead, Jim!", L"one or more toolbox entries failed to load! \nsee StackEditor.log in your Orbiter/StackEditor directory for details.");
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
	rect<s32> tbxrect = toolBoxList[0].getAbsoluteClippingRect();

	
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
	camera = new ShipyardCamera(vector3d<f32>(0, 0, 0), 30, device, smgr->addCameraSceneNode());

	smgr->setAmbientLight(SColor(150,150,150,150));
	scene::ILightSceneNode *light = smgr->addLightSceneNode(0, core::vector3df(200, 282, 200),
		video::SColorf(0.2f, 0.2f, 0.2f));
	light->setRadius(2000);


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

		//checking toolbox for vessels to be created
		ToolboxData* toolboxData = toolboxes[activetoolbox]->checkCreateVessel();
		if (toolboxData != NULL)
		{
			VesselData *createVessel = dataManager.GetGlobalConfig(toolboxData->configFileName, driver);
			if (createVessel != NULL)
			{
				addVessel(createVessel);
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

void Shipyard::addVessel(VesselData* vesseldata, bool snaptocursor)
{
	//add the vessel
	VesselSceneNode* newvessel = new VesselSceneNode(vesseldata, smgr->getRootSceneNode(), smgr, VESSEL_ID);
	//register for fast vessel identification
	registerVessel(newvessel);

	if (snaptocursor)
	{
		//move the newly created vessel to the mousecursor and make it selected
		moveVesselToCursor(newvessel);
		selectedVesselStack = new VesselStack(newvessel);
		setupSelectedStack();
	}

	lastSpawnedVessel = vesseldata;

	//remove focus from the element so no vessels can be spawned while the current one is still being selected
	guiEnv->removeFocus(toolboxes[activetoolbox]);
	cursorOnGui = false;

}

void Shipyard::registerVessel(VesselSceneNode* node)
{
	vessels.push_back(node);
	for (vector<OrbiterDockingPort>::iterator i = node->dockingPorts.begin(); i != node->dockingPorts.end(); ++i)
	{
		dockportmap.insert(make_pair((*i).portNode, node));
	}
	if (selectedVesselStack != 0)
		setupSelectedStack(); //this is needed to show available docking ports if we copied
}

void Shipyard::registerVessels(const std::vector<VesselSceneNode*>& nodes)
{
	for (UINT i = 0; i < nodes.size(); ++i)
	{
		registerVessel(nodes[i]);
	}
}

void Shipyard::deregisterVessel(VesselSceneNode* node)
{
	//Remove node from vessels list by value
	vessels.erase(std::remove(vessels.begin(), vessels.end(), node), vessels.end());
	//remove all of the docking ports in this node
	for (UINT i = 0; i < node->dockingPorts.size(); ++i)
	{
		dockportmap.erase(node->dockingPorts[i].portNode);
	}
}

void Shipyard::deregisterVessels(const std::vector<VesselSceneNode*>& nodes)
{
	for (UINT i = 0; i < nodes.size(); ++i)
	{
		deregisterVessel(nodes[i]);
	}
}

void Shipyard::deregisterVessels(VesselStack* stack)
{
	for (UINT i = 0; i < stack->numVessels(); ++i)
	{
		deregisterVessel(stack->getVessel(i));
	}
}

void Shipyard::moveVesselToCursor(VesselSceneNode* vessel)
//moves a VesselSceneNode to the mousecursor. used when a new vessel is created
{
	//estimating a radius based on the maximum extent of the vessel
	vector3df extent = vessel->getBoundingBox().getExtent();
	float radius = max(extent.X, max(extent.Y, extent.Z)) + 20;
	vessel->setPosition(camera->getCursorPosAtRadius(radius));
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
	//also, some events fire before setupDevice() is called, so we have to make sure that it has already been initialised
	if (device)
	{
		if (activetoolbox != toolBoxList->getSelected())
		{
			switchToolBox();
		}
	}


	switch (event.EventType)
	{
	case EET_GUI_EVENT:
		processGuiEvent(event);
		break;
	case EET_KEY_INPUT_EVENT:
		processKeyboardEvent(event);
		break;
	case EET_MOUSE_INPUT_EVENT:
		processMouseEvent(event);
		break;
	}
	return false;
}

bool Shipyard::processGuiEvent(const SEvent &event)
{
	switch (event.GUIEvent.EventType)
	{
		//message boxes and file dialogs////////////////////////////////

		case gui::EGET_FILE_SELECTED:
		{
			std::wstring uniString = std::wstring(((gui::IGUIFileOpenDialog*)event.GUIEvent.Caller)->getFileName());
			std::string fullfilename = std::string(uniString.begin(), uniString.end());

			if (event.GUIEvent.Caller->getID() == TBXOPENFILE)
			{
				std::string filename = fullfilename.substr(Helpers::workingDirectory.length() + 16);
				//create a new toolbox entry
				bool success = toolboxes[UINT(toolBoxList->getSelected())]->addElement(dataManager.GetGlobalToolboxData(filename, device->getVideoDriver()));
				if (!success)
				//pop a message that the vessel could not be loaded
				{
					guiEnv->addMessageBox(L"He's dead, Jim!", L"This vessel can't be loaded by StackEditor! \nThis is most likely because the config file doesn't specify docking ports, doesn't specify a mesh or specifies a mesh that doesn't exist.");
					Helpers::resetDirectory();
				}
				else
				{
					//reopen file dialog to make multiple selections easier
					guiEnv->addFileOpenDialog(L"Select Config File", true, 0, -1);
				}
			}
			else if (event.GUIEvent.Caller->getID() == SESSIONOPENFILE)
			{
				if (loadSession(fullfilename))
				{
					//extract the actual session name from the file path
					std::vector<std::string> tokens;
					Helpers::tokenize(fullfilename, tokens, "/\\.");
					session = tokens[tokens.size() - 2];
					std::string newcaption = "Orbiter Shipyard - " + session;
					device->setWindowCaption(std::wstring(newcaption.begin(), newcaption.end()).c_str());
				}
				Helpers::resetDirectory();
			}
			cursorOnGui = false;
			dialogOpen = false;
		}
		break;
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
			else if (event.GUIEvent.Caller->getID() == NAMESESSION)
			{
				IGUIEditBox *sessionnamebox = (gui::IGUIEditBox*)event.GUIEvent.Caller->getElementFromId(SESSIONNAMEENTRY, true);
				session = std::string(stringc(sessionnamebox->getText()).c_str());
				saveSession(session);
			}
			else if (event.GUIEvent.Caller->getID() == SESSIONLOADCONFIRM)
			{
				IGUIFileOpenDialog *dlg = guiEnv->addFileOpenDialog(L"Select Config File", true, 0, SESSIONOPENFILE, false, "StackEditor/Sessions");
			}
			else if (event.GUIEvent.Caller->getID() == NAMESTACK)
			{
				IGUIEditBox *stacknamebox = (gui::IGUIEditBox*)event.GUIEvent.Caller->getElementFromId(NAMESTACKENTRY, true);
				std::string stackname = std::string(stringc(stacknamebox->getText()).c_str());
				if (stackname != "")
				{
					//put the stack in the export data
					_exportdata->locked = true;
					_exportdata->stack = selectedVesselStack;
					_exportdata->name = stackname;
					_exportdata->exporting = true;
					_exportdata->locked = false;
					//hold this thread until export finishes
					while (_exportdata->exporting){};
					
					//put the stack down. Easiest way to avoid motion mayhem afterwards.
					selectedVesselStack->changeDockingPortVisibility(false, false);
					//hide all the rest of the empty docking ports
					for (unsigned int i = 0; i < vessels.size(); i++)
					{
						if (!selectedVesselStack->isVesselInStack(vessels[i]))
						{
							vessels[i]->changeDockingPortVisibility(false, false);
						}
					}

					delete selectedVesselStack;
					selectedVesselStack = NULL;
				}
			}
				
			cursorOnGui = false;
			dialogOpen = false;
			return true;
		}
		break;
		case gui::EGET_MESSAGEBOX_CANCEL:
			cursorOnGui = false;
			dialogOpen = false;
		break;
		case gui::EGET_FILE_CHOOSE_DIALOG_CANCELLED:
			isOpenDialogOpen = false;
			cursorOnGui = false;
			dialogOpen = false;
			Helpers::resetDirectory();
		break;


		//permanent gui elements///////////////////////////////////////////////

		case gui::EGET_BUTTON_CLICKED:
			if (event.GUIEvent.Caller->getID() == LOADSESSION)
			{
				if (vessels.size() == 0)
				//spawn open dialog
				{
					IGUIFileOpenDialog *dlg = guiEnv->addFileOpenDialog(L"Select Config File", true, 0, SESSIONOPENFILE, false, "StackEditor/Sessions");
					dialogOpen = true;
				}
				else
				//spawn confirmation message
				{
					IGUIWindow *msgBx = guiEnv->addMessageBox(L"Sir, are you absolutely sure? It does mean changing the bulb.", L"Any unsaved changes in this session will be lost. Proceed anyways?", true, EMBF_OK | EMBF_CANCEL, 0, SESSIONLOADCONFIRM);
					dialogOpen = true;
				}
			}
			else if (event.GUIEvent.Caller->getID() == SAVESESSIONAS || event.GUIEvent.Caller->getID() == SAVESESSION && session.compare("unnamed") == 0)
			{
			//spawn naming dialog
				IGUIWindow *msgBx = guiEnv->addMessageBox(L"session name", L"please enter a name for this session", true, EMBF_OK | EMBF_CANCEL, 0, NAMESESSION);
				msgBx->setMinSize(dimension2du(300, 150));
				guiEnv->addEditBox(L"session name", rect<s32>(20, 100, 280, 130), true, msgBx, SESSIONNAMEENTRY);
				dialogOpen = true;
			}
			else if (event.GUIEvent.Caller->getID() == SAVESESSION)
			{
				saveSession(session);
			}
			break;
		case gui::EGET_MENU_ITEM_SELECTED:
			if (event.GUIEvent.Caller->getID() == TOOLBOXCONTEXT)
			//toolbox context menu
			{
				s32 menuItem = ((gui::IGUIContextMenu*)event.GUIEvent.Caller)->getSelectedItem();
				if (menuItem == 0)
				//open file dialog to add new element
				{
					IGUIFileOpenDialog *dlg = guiEnv->addFileOpenDialog(L"Select Config File", true, 0, TBXOPENFILE, false, "config\\vessels");
					dialogOpen = true;
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
					dialogOpen = true;
				}
				if (menuItem == 3)
				//pop a confirmation message for deletion
				{
					if (toolBoxList->getItemCount() > 1)
					{
						IGUIWindow *msgBx = guiEnv->addMessageBox(L"Sir, are you absolutely sure? It does mean changing the bulb.",
							L"this will permanently delete the currently active toolbox. \n Are you sure?",
							true, EMBF_OK | EMBF_CANCEL, 0, TBXDELETE);
						dialogOpen = true;
					}
					else
					//there's only one toolbox left. If we delete that, there will be mayhem.
					{
						IGUIWindow *msgBx = guiEnv->addMessageBox(L"I'm afraid I can't do that, Dave...",
							L"There is only one toolbox left, and Stackeditor needs at least one to function properly! \n \n If you really need to delete this toolbox, create an empty one first.");
						dialogOpen = true;
					}
				}
			}
		break;

		case gui::EGET_ELEMENT_HOVERED:
			if (!camera->IsActionInProgress() && selectedVesselStack == NULL)
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
	return false;
}

bool Shipyard::processKeyboardEvent(const SEvent &event)
{

	//it's a key, store it
	isKeyDown[event.KeyInput.Key] = event.KeyInput.PressedDown;

	if (dialogOpen)
	{
		return false;
	}

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
			//stack export triggered by ctrl-o. Only has effect in orbiter integrated version
		case KEY_KEY_O:
			if (_exportdata && isKeyDown[EKEY_CODE::KEY_LCONTROL] || _exportdata && isKeyDown[EKEY_CODE::KEY_RCONTROL])
			{
				//throw up a naming dialog
				IGUIWindow *msgBx = guiEnv->addMessageBox(L"I christen thee...", L"please enter a name for your stack.\nThe currently selected vessel will bear the stack name, all other vessels will get a number tagged to them.\n\nAnd remember, orbiter vessels should never have spaces in their filenames!", true, EMBF_OK | EMBF_CANCEL, 0, NAMESTACK);
				msgBx->setMinSize(dimension2du(300, 250));
				guiEnv->addEditBox(L"stack", rect<s32>(20, 200, 280, 230), true, msgBx, NAMESTACKENTRY);
				dialogOpen = true;
			}
			break;

		default:
			break;
		}
		if (didWeRotate)
		{
			//reset the move reference, as we screwed up the initial positions
			selectedVesselStack->setMoveReference(returnMouseRelativePos());
		}

		//check for splitting stack or copying
		if (event.KeyInput.PressedDown)
		{
			switch (event.KeyInput.Key)
			{
			case KEY_TAB:
				areSplittingStack = !areSplittingStack;
				if (areSplittingStack)
				{
					selectedVesselStack->changeDockingPortVisibility(false, false);
					selectedVesselStack->showFirstNodeForSplitting();
				}
				else
				{
					selectedVesselStack->resetFirstNode();
					selectedVesselStack->changeDockingPortVisibility(true, false);
				}
				break;

			case KEY_KEY_C:
				if (selectedVesselStack != 0 && isKeyDown[EKEY_CODE::KEY_LCONTROL])
				{
					registerVessels(VesselStackOperations::copyStack(selectedVesselStack, smgr));
				}
				break;
			case KEY_DELETE:
				if (selectedVesselStack != 0)
				{
					deregisterVessels(selectedVesselStack);
					VesselStackOperations::deleteStack(selectedVesselStack);
					//remove stack reference
					selectedVesselStack = 0;
				}
				break;
			default:
				break;
			}
		}
	}
	return false;
}

bool Shipyard::processMouseEvent(const SEvent &event)
{
	//return if cursor is over the GUI, so the GUI gets the event
	if (cursorOnGui || dialogOpen)
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
		//if we have a selected node, deselect it or check for stack splitting
		if (selectedVesselStack != 0)
		{
			if (areSplittingStack)
			{
				//try seeing if we clicked on a docking node
				scene::ISceneNode* selectedNode = collisionManager->getSceneNodeFromScreenCoordinatesBB(
					device->getCursorControl()->getPosition(), HELPER_ID, true);
				if (selectedNode->getID() == HELPER_ID)
				{
					//get vessel and port
					VesselSceneNode* nodeVessel = (VesselSceneNode*)selectedNode->getParent();
					OrbiterDockingPort* selectedDockingPort = nodeVessel->dockingPortHelperNodeToOrbiter(selectedNode);

					//if it is docked, undock it to split the stack
					//and regenerate stack
					if (selectedDockingPort->docked)
					{
						//before splitting, reset docking port node visibility
						selectedVesselStack->resetFirstNode();
						selectedVesselStack->changeDockingPortVisibility(false, false);
						VesselStackOperations::splitStack(selectedDockingPort);
						delete selectedVesselStack;
						selectedVesselStack = new VesselStack(nodeVessel);
						areSplittingStack = false;
						setupSelectedStack();
					}
				}
			}
			else
			{

				//try docking this node
				scene::ISceneNode* selectedNode = collisionManager->getSceneNodeFromScreenCoordinatesBB(
					device->getCursorControl()->getPosition(), DOCKPORT_ID, true);
				if (selectedNode->getID() == DOCKPORT_ID)
				{
					selectedVesselStack->checkForSnapping(dockportmap.find(selectedNode)->second, selectedNode, true);
				}

				//hide docking ports
				selectedVesselStack->changeDockingPortVisibility(false, false);
				//hide all the rest of the empty docking ports
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
		}
		else if (isKeyDown[EKEY_CODE::KEY_LSHIFT])
			//spawn last created vessel
		{
			if (lastSpawnedVessel != NULL)
			{
				addVessel(lastSpawnedVessel);
			}
			return true;
		}
		else
		{
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
			setupSelectedStack();
			//return true if we got a vessel stack
			return (selectedVesselStack != 0);
		}
		break;
	}
	case EMIE_MOUSE_MOVED:
		//see if we have a node AND the camera isn't rotating or translating 
		//AND we aren't selecting a node to split the stack
		if (selectedVesselStack != 0 && !areSplittingStack && !camera->IsActionInProgress() && !isKeyDown[EKEY_CODE::KEY_LCONTROL])
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
			else if (selectedVesselStack->isSnaped())
				//there's no dockport nearby, release the stack from snap and set up a new move reference
			{
				selectedVesselStack->unSnap(returnMouseRelativePos());
			}
		}
		//update camera position
		camera->UpdatePosition(event.MouseInput.X, event.MouseInput.Y, isKeyDown[EKEY_CODE::KEY_LCONTROL]);
		break;
	case EMIE_MOUSE_WHEEL:
		camera->UpdateRadius(event.MouseInput.Wheel);
		break;
	}
	return false;
}

void Shipyard::setupSelectedStack()
//sets up move reference and ports for the selected vessel stack
{
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
			toolboxes[toolboxes.size() - 1]->finishedLoading();
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


void Shipyard::saveSession(std::string filename)
{
	std::string fullpath = Helpers::workingDirectory + "\\StackEditor\\Sessions\\" + filename + ".ses";
	ofstream file(fullpath);
	//we'll note the respective indices of vessels and dockports to have an easier time saving the docking connections
	std::map<VesselSceneNode*, UINT> indices_by_vessels;
	std::map<OrbiterDockingPort*, UINT> indices_by_dockports;

	for (UINT i = 0; i < vessels.size(); ++i)
	{
		//save VesselSceneNodes to file and remember their indices
		vessels[i]->saveToSession(file);
		indices_by_vessels.insert(std::make_pair(vessels[i], i));
		//walk through and note dockports
		for (UINT j = 0; j < vessels[i]->dockingPorts.size(); ++j)
		{
			indices_by_dockports.insert(std::make_pair(&vessels[i]->dockingPorts[j], j));
		}
	}

	file << "DOCKINFO =";
	for (UINT i = 0; i < vessels.size(); ++i)
	{
		for (UINT j = 0; j < vessels[i]->dockingPorts.size(); ++j)
		{
			if (!vessels[i]->dockingPorts[j].docked)
			//write -1 for docked vessel and port
			{
				file << " -1:-1";
			}
			else
			//write index of connecting vessel followed by index of dockport on connected vessel
			{
				file << " " << indices_by_vessels[vessels[i]->dockingPorts[j].dockedTo->parent] 
					<< ":" << indices_by_dockports[vessels[i]->dockingPorts[j].dockedTo];
			}
		}
	}
	file.close();
	std::string newcaption = "Orbiter Shipyard - " + filename;
	device->setWindowCaption(std::wstring(newcaption.begin(), newcaption.end()).c_str());
	session = filename;
}


bool Shipyard::loadSession(std::string path)
{
	clearSession();
	ifstream file(path.c_str());
	std::vector<std::string> tokens;
	while (Helpers::readLine(file, tokens))
	{
		if (tokens.size() == 0)
		{
			continue;
		}

		if (tokens[0].compare("VESSEL_BEGIN") == 0)
		{
			tokens.clear();
			Helpers::readLine(file, tokens);
			if (tokens[0].compare("FILE") != 0)
			//there's no file for the vessel, throw error and abort
			{
				Helpers::writeToLog(std::string("Error: No FILE declared for vessel, unable to load session"));
				guiEnv->addMessageBox(L"He's dead, Jim!", L"No FILE declared for vessel, unable to load session");
				return false;
			}

			addVessel(dataManager.GetGlobalConfig(tokens[1], device->getVideoDriver()), false);
			if (!vessels[vessels.size() - 1]->loadFromSession(file))
			{
				guiEnv->addMessageBox(L"He's dead, Jim!", L"error while loading session");
				return false;
			}
		}
		else if (tokens[0].compare("DOCKINFO") == 0)
		//establish docking connections
		{
			UINT tokenidx = 1;
			for (UINT i = 0; i < vessels.size(); ++i)
			{
				for (UINT j = 0; j < vessels[i]->dockingPorts.size(); ++j)
				{
					//take appart the token containing the index of the docked vessel and its dockport
					vector<std::string> connection;
					Helpers::tokenize(tokens[tokenidx], connection, ":");
					UINT vesselidx = Helpers::stringToInt(connection[0]);
					UINT portidx = Helpers::stringToInt(connection[1]);
					if (vesselidx != -1 && portidx != -1)
					{
						//we have a connection, update the dockport of the vessel
						vessels[i]->dockingPorts[j].docked = true;
						vessels[i]->dockingPorts[j].dockedTo = &vessels[vesselidx]->dockingPorts[portidx];
					}
					tokenidx++;
				}
			}
		}
		tokens.clear();
	}
	file.close();
	return true;
}

void Shipyard::clearSession()
{
	for (UINT i = 0; i < vessels.size(); ++i)
	{
		vessels[i]->remove();
		vessels[i]->drop();
	}
	vessels.clear();
}