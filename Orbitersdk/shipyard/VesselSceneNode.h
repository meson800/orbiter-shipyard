#include <irrlicht.h>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

#include "OrbiterMesh.h"
#include "Helpers.h"
#include "OrbiterDockingPort.h"

using namespace irr;
using namespace std;

class VesselSceneNode : public scene::ISceneNode
{
public:
	VesselSceneNode(string configFilename, scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id);

private:
	vector<OrbiterDockingPort> dockingPorts;
	OrbiterMesh vesselMesh;
};