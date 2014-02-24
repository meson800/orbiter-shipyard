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
	OrbiterMesh(string meshFilename, video::IVideoDriver* driver, scene::ISceneManager* smgr);
	void setupMesh(string meshFilename, video::IVideoDriver* driver, scene::ISceneManager* smgr);
	core::aabbox3d<f32> boundingBox;
	vector<video::SMaterial> materials;
	vector<video::ITexture*> textures;
	vector<OrbiterMeshGroup> meshGroups;

private:
	void setupNormals(int meshGroup);
};