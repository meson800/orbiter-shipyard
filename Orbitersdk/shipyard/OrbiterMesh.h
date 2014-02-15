#include <irrlicht.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "OrbiterMeshGroup.h"

using namespace irr;
using namespace std;

class OrbiterMesh
{
public:
	OrbiterMesh(string meshFilename);

public:
	vector<video::SMaterial> materials;
	vector<OrbiterMeshGroup> meshGroups;
};