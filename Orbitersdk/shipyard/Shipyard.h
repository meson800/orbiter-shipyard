#pragma once

#include <irrlicht.h>
#include <vector>
#include <string>

#include "resource.h"
#include "VesselSceneNode.h"
#include "VesselStack.h"
//#include "CSceneNodeAnimatorCameraCustom.h"
#include "ShipyardCamera.h"
#include "DataManager.h"
#include "SE_ToolBox.h"
#include "SE_PhotoStudio.h"


#define cameraRotateSpeed = .1;

using namespace irr;

class Shipyard : public IEventReceiver
{
public:
	Shipyard();
	~Shipyard();
	void setupDevice(IrrlichtDevice * _device, std::string toolboxSet, DataManager *datamanager);
	void loop();
	bool OnEvent(const SEvent & event);

private:
	bool processGuiEvent(const SEvent &event);
	bool processKeyboardEvent(const SEvent &event);
	bool processMouseEvent(const SEvent &event);

	core::aabbox3d<f32> returnOverallBoundingBox();
	void centerCamera();

	core::vector3df returnMouseRelativePos();
	void moveVesselToCursor(VesselSceneNode* vessel);

	std::string tbxSet;
	gui::IGUIEnvironment* guiEnv;
	std::vector<VesselSceneNode*> vessels;
	std::map<ISceneNode*, VesselSceneNode*> dockportmap;					//maps dockport nodes to vessels for performance when checking for mouse/dockport overlap
	bool isKeyDown[KEY_KEY_CODES_COUNT];
	bool isOpenDialogOpen;
	IrrlichtDevice * device;
	//scene::ICameraSceneNode* camera;
	ShipyardCamera* camera;
	VesselStack* selectedVesselStack;
	void setupSelectedStack();
	scene::ISceneCollisionManager* collisionManager;
	scene::ISceneManager* smgr;
	DataManager *dataManager;
	void addVessel(VesselData* vesseldata, bool snaptocursor = true);										//adds a new vessel to the scene
	bool cursorOnGui;															//registers when the cursor is over a GUI element, so events can be passed on
	vector<CGUIToolBox*> toolboxes;
	IGUIListBox *toolBoxList;
	int activetoolbox;
	SColor scenebgcolor;
	bool loadToolBoxes();
	void saveToolBoxes();
	VesselData *lastSpawnedVessel;
	void switchToolBox();

	void saveSession(std::string filename);
	bool loadSession(std::string path);
	void clearSession();
	std::string session;
};