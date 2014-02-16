#include <irrlicht.h>

using namespace irr;

struct OrbiterDockingPort
{
	core::vector3d<f32> position;
	core::vector3d<f32> approachDirection;
	core::vector3d<f32> referenceDirection;
};