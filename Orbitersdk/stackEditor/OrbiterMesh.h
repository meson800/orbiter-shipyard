#pragma once

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
	OrbiterMesh();
	OrbiterMesh(std::string meshFilename, video::IVideoDriver* driver, scene::ISceneManager* smgr);
	bool setupMesh(std::string meshFilename, video::IVideoDriver* driver);
	core::aabbox3d<f32> boundingBox;
	vector<video::SMaterial> materials;
	vector<video::ITexture*> textures;
	vector<OrbiterMeshGroup> meshGroups;
	void getOuterDimensions(core::vector3df &max, core::vector3df &min);

private:
	void setupNormals(int meshGroup);
};