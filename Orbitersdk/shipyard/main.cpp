#include <irrlicht.h>
#include "Shipyard.h"

#ifdef _IRR_WINDOWS_
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

using namespace irr;

int main()
{
	// create a NULL device to detect screen resolution
	IrrlichtDevice *nulldevice = createDevice(video::EDT_NULL);

	core::dimension2d<u32> deskres = nulldevice->getVideoModeList()->getDesktopResolution();

	nulldevice->drop();

	Shipyard shipyard = Shipyard();

	IrrlichtDevice *device = createDevice(video::EDT_DIRECT3D9, deskres, 32, false, false, false, &shipyard);
	if (!device)
		return 1;

	//set caption
	device->setWindowCaption(L"Orbiter Shipyard");

	//pass it off to Shipyard
	shipyard.setupDevice(device);
	//and run!
	shipyard.loop();
	device->drop();
	
	return 0;
} 