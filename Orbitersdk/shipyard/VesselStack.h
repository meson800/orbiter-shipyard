#pragma once

#include <vector>
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
	void changeDockingPortVisibility(bool showEmpty, bool showDocked);
	int numVessels();
	VesselSceneNode* getVessel(int index);
	bool isVesselInStack(VesselSceneNode* vessel);
	int getIndexOfVessel(VesselSceneNode* vessel);
	void unSnap(core::vector3df refPos);
	bool isSnaped();
	OrbiterDockingPort* dockingPortSceneNodeToOrbiter(scene::ISceneNode* sceneNode);
	UINT getStackSize();
	void showFirstNodeForSplitting();
	void resetFirstNode();
private:
	//recursive helper to init the stack
	void createStackHelper(VesselSceneNode* startingVessel, OrbiterDockingPort* fromPort);
	void snapStack(int srcvesselidx, int srcdockportidx = -1, OrbiterDockingPort *tgtport = NULL);
	std::vector<core::vector3df> previousPositions;
	core::vector3df moveReference, currentStackLocation;
	std::vector<VesselSceneNode*> nodes;
	std::vector<ISceneNode*> dockportnodes;
	bool issnaped;
};