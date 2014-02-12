#include <irrlicht.h>

using namespace irr;

class Shipyard
{
public:
	Shipyard(IrrlichtDevice * _device);
	void run();

private:
	IrrlichtDevice * device;
};