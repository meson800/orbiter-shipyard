#include <iostream>
#include <vector>
#include <irrlicht.h>
#include "Helpers.h"

using namespace irr;

struct OrbiterMeshGroup
{
	std::vector<video::S3DVertex> vertices;
	std::vector<int> triangleList;
	int materialIndex;
	int textureIndex;
};