//Copyright (c) 2015 Christopher Johnstone(meson800) and Benedict Haefeli(jedidia)
//The MIT License - See ../../LICENSE for more info
#pragma once

#include "Log.h"

#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include "VesselSceneNode.h"
#include "OrbiterDockingPort.h"

class VesselStack
{
public:
	VesselStack(VesselSceneNode* startingVessel);
	void rotateStack(core::vector3df relativeRot);
	void rotateStack(core::quaternion relativeRot);
//	void rotateStackAroundVessel(core::vector3df relativeRot, VesselSceneNode *vessel = NULL);
	void setMoveReference(core::vector3df refPos);
	void moveStackReferenced(core::vector3df movePos);
	void moveStackRelative(core::vector3df movePos);
	void checkForSnapping(VesselSceneNode* vessel, ISceneNode* dockportnode, bool dock=false);
	void snapStack(int srcvesselidx, int srcdockportidx = -1, OrbiterDockingPort *tgtport = NULL);
	void changeDockingPortVisibility(bool showEmpty, bool showDocked);
	UINT numVessels();
	VesselSceneNode* getVessel(UINT index);
	bool isVesselInStack(VesselSceneNode* vessel);
	int getIndexOfVessel(VesselSceneNode* vessel);
	void unSnap(core::vector3df refPos);
	bool isSnaped();
	OrbiterDockingPort* dockingPortSceneNodeToOrbiter(scene::ISceneNode* sceneNode);
	UINT getStackSize();
	void showFirstNodeForSplitting();
	void resetFirstNode();

    std::string toString();
private:
	//recursive helper to init the stack
	void createStackHelper(VesselSceneNode* startingVessel);
	std::vector<core::vector3df> previousPositions;
	core::vector3df moveReference, currentStackLocation;
	std::vector<VesselSceneNode*> nodes;
	std::vector<ISceneNode*> dockportnodes;
	bool issnaped;
};
