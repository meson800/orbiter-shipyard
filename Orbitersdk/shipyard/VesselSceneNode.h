#pragma once

#include <irrlicht.h>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <stdexcept>

#include "OrbiterMesh.h"
#include "Helpers.h"
#include "OrbiterDockingPort.h"
#include "resource.h"
#include "DataManager.h"

using namespace irr;
using namespace std;

struct VesselSceneNodeState
{
    VesselData* vesData;
    UINT uid;
    core::vector3df pos, rot;
    std::string orbiterName;
    std::vector<DockingPortStatus> dockingStatus;
};

class VesselSceneNode : public scene::ISceneNode
{
public:
	VesselSceneNode(VesselData *vesData, scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id, 
        UINT _uid = 0, bool deferRegistration=false); 
    VesselSceneNode(const VesselSceneNodeState& state, scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id);
    ~VesselSceneNode();

	virtual void OnRegisterSceneNode();
	virtual void render();
	virtual void drawDockingPortLines(video::IVideoDriver* driver);
	virtual const core::aabbox3d<f32>& getBoundingBox() const;
	virtual u32 getMaterialCount();
	virtual video::SMaterial& getMaterial(u32 i);
	void setupDockingPortNodes();
	void changeDockingPortVisibility(bool showEmpty, bool showDocked, bool showHelper = false);
	void snap(OrbiterDockingPort& ourPort, OrbiterDockingPort& theirPort);
	void dock(OrbiterDockingPort& ourPort, OrbiterDockingPort& theirPort);
    void dock(UINT ourPortNum, UINT otherVesselUID, UINT otherPortID);
	core::vector3df returnRotatedVector(const core::vector3df& vec);
	VesselData* returnVesselData();
	void setTransparency(bool transparency);

	OrbiterDockingPort* dockingPortSceneNodeToOrbiter(scene::ISceneNode* sceneNode);
	OrbiterDockingPort* dockingPortHelperNodeToOrbiter(scene::ISceneNode* sceneNode);
	void saveToSession(ofstream &file);
	bool loadFromSession(ifstream &file);

    VesselSceneNodeState saveState();
    void loadState(const VesselSceneNodeState& state);

	std::string getClassName();

    UINT getUID();
	std::string getOrbiterName();
	void setOrbiterName(std::string name);

	vector<OrbiterDockingPort> dockingPorts;

    class UID_Mismatch : public std::exception {};

private:
    UINT uid;
    static UINT next_uid;
	scene::ISceneManager* smgr;
	OrbiterMesh *vesselMesh;
	VesselData *vesselData;
	void setupDockingPortNode(IMeshSceneNode *node);
	bool transparent;
	std::string orbitername;
};