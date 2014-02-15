#include <iostream>
#include <vector>
#include <tuple>
#include <irrlicht.h>
#include "Helpers.h"

using namespace irr;

struct OrbiterMeshGroup
{
	std::vector<video::S3DVertex> vertices;
	std::vector<std::tuple<int,int,int>> triangleList;
	int materialIndex;
	int textureIndex;
};