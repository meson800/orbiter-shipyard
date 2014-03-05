#define STRICT 1
#define ORBITER_MODULE

#include <irrlicht.h>
#include <algorithm>
#include "orbitersdk.h"
#include "Shipyard.h"
#include "Helpers.h"


#ifdef _IRR_WINDOWS_
#pragma comment(linker, "/subsystem:windows")
#endif

using namespace irr;

DWORD g_dwCmd;
void OpenSE(void *context);

DLLCLBK void InitModule(HINSTANCE hDLL)
{

	// To allow the user to open our new dialog box, we create
	// an entry in the "Custom Functions" list which is accessed
	// in Orbiter via Ctrl-F4.
	g_dwCmd = oapiRegisterCustomCmd("StackEditor",
		"Opens StackEditor in an external window",
		OpenSE, NULL);

}

DLLCLBK void ExitModule(HINSTANCE hDLL)
{
	oapiUnregisterCustomCmd(g_dwCmd);
}

void OpenSE(void *context)
{

	// create a NULL device to detect screen resolution
	IrrlichtDevice *nulldevice = createDevice(video::EDT_NULL);

//	core::dimension2d<u32> deskres = nulldevice->getVideoModeList()->getDesktopResolution();
	core::dimension2d<u32> deskres = core::dimension2d<u32>(1366, 710);
	nulldevice->drop();

	Shipyard shipyard = Shipyard();

	IrrlichtDevice *device = createDevice(video::EDT_DIRECT3D9, deskres, 32, false, false, false, &shipyard);
	if (!device)
		return;

	//set caption
	device->setWindowCaption(L"Orbiter Shipyard");

	//set the working directory
	std::string directory = device->getFileSystem()->getWorkingDirectory().c_str();
	std::replace(directory.begin(), directory.end(), '/', '\\');
	Helpers::workingDirectory = directory;

	//pass it off to Shipyard
	shipyard.setupDevice(device);
	//and run!
	shipyard.loop();
	device->drop();
	
	return;
} 