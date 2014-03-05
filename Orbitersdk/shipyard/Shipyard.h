#pragma once

#include <irrlicht.h>
#include <vector>
#include <string>

#include "resource.h"
#include "VesselSceneNode.h"
#include "CSceneNodeAnimatorCameraCustom.h"
#include "DataManager.h"
#include "SE_ToolBox.h"

#define cameraRotateSpeed = .1;

using namespace irr;

class Shipyard : public IEventReceiver
{
public:
	Shipyard();
	~Shipyard();
	void setupDevice(IrrlichtDevice * _device, std::string toolboxSet);
	void loop();
	bool OnEvent(const SEvent & event);

private:
	core::vector3df returnMouseRelativePos();
	void checkNodeForSnapping(VesselSceneNode* node);
	
	std::string tbxSet;
	gui::IGUIEnvironment* guiEnv;
	std::vector<VesselSceneNode*> vessels;
	bool isKeyDown[KEY_KEY_CODES_COUNT];
	bool isOpenDialogOpen;
	IrrlichtDevice * device;
	scene::ICameraSceneNode* camera;
	scene::ISceneNode * selectedNode;
	scene::ISceneCollisionManager* collisionManager;
	scene::ISceneManager* smgr;
	core::vector3df originalMouse3DPos;
	core::vector3df originalNodePosition;
	DataManager dataManager;

	bool cursorOnGui;				//registers when the cursor is over a GUI element, so events can be passed on
	vector<CGUIToolBox*> toolboxes;
	IGUIListBox *toolBoxList;

	bool loadToolBoxes();
	void saveToolBoxes();

	void switchToolBox();
};