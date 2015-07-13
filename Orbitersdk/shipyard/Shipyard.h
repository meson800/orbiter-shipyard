#pragma once

#include <irrlicht.h>
#include <vector>
#include <map>
#include <string>
#include <stack>
#include <algorithm>

#include "resource.h"
#include "VesselSceneNode.h"
#include "VesselStack.h"
#include "VesselStackOperations.h"
#include "SE_State.h"
//#include "CSceneNodeAnimatorCameraCustom.h"
#include "ShipyardCamera.h"
#include "DataManager.h"
#include "SE_ToolBox.h"
#include "SE_PhotoStudio.h"
#include "StackExportStructs.h"



#define cameraRotateSpeed = .1;

using namespace irr;

class Shipyard : public IEventReceiver
{
public:
	Shipyard(ExportData *exportdata = NULL, ImportData *importdata = NULL);
	~Shipyard();
	void setupDevice(IrrlichtDevice * _device, std::string toolboxSet);
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

    std::map<UINT, VesselSceneNode*> uidVesselMap;
	bool isKeyDown[KEY_KEY_CODES_COUNT];
	bool isOpenDialogOpen;
	IrrlichtDevice * device;
	//scene::ICameraSceneNode* camera;
	ShipyardCamera* camera;
	VesselStack* selectedVesselStack;
	void setupSelectedStack();
	scene::ISceneCollisionManager* collisionManager;
	scene::ISceneManager* smgr;
	DataManager dataManager;
	VesselSceneNode *addVessel(VesselData* vesseldata, bool snaptocursor = true);		//adds a new vessel to the scene

    void setAllDockingPortVisibility(bool showEmpty, bool showDocked);
	
	bool cursorOnGui;															//registers when the cursor is over a GUI element, so events can be passed on
	bool dialogOpen;															//true while dialog windows are open
	vector<CGUIToolBox*> toolboxes;
	IGUIListBox *toolBoxList;
	int activetoolbox;
	SColor scenebgcolor;
	bool loadToolBoxes();
	void saveToolBoxes();
	VesselData *lastSpawnedVessel;
	void switchToolBox();

	bool areSplittingStack;

	void saveSession(std::string filename);
	bool loadSession(std::string path);
	void clearSession();
	void importStack();

    std::stack<SE_DiffState> undoStack;
    std::stack<SE_DiffState> redoStack;
    void undo();
    void redo();
    void pushUndoStack();
    SE_GlobalState lastGlobalState;

	std::string session;
	ExportData *_exportdata;
	ImportData *_importdata;
};