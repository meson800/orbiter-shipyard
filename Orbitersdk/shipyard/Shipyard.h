#pragma once

#include <irrlicht.h>
#include <vector>
#include <string>

#include "resource.h"
#include "VesselSceneNode.h"
#include "VesselStack.h"
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
	core::aabbox3d<f32> returnOverallBoundingBox();
	void centerCamera();

	core::vector3df returnMouseRelativePos();
	
	std::string tbxSet;
	gui::IGUIEnvironment* guiEnv;
	std::vector<VesselSceneNode*> vessels;
	bool isKeyDown[KEY_KEY_CODES_COUNT];
	bool isOpenDialogOpen;
	IrrlichtDevice * device;
	scene::ICameraSceneNode* camera;
	VesselStack * selectedVesselStack;
	scene::ISceneCollisionManager* collisionManager;
	scene::ISceneManager* smgr;
	DataManager dataManager;

	bool cursorOnGui;				//registers when the cursor is over a GUI element, so events can be passed on
	vector<CGUIToolBox*> toolboxes;
	IGUIListBox *toolBoxList;

	bool loadToolBoxes();
	void saveToolBoxes();

	void switchToolBox();
};