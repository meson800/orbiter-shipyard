#include "Common.h"
#include <algorithm>
#include "StackEditor.h"
#include "Helpers.h"

#ifdef _IRR_WINDOWS_
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

using namespace irr;



int main()
{

	//loading configuration from StackEditor.cfg
	CONFIGPARAMS params = Helpers::loadConfigParams();

	if (params.windowres == dimension2d<u32>(0, 0))
	//resolution not specified, create a NULL device to detect screen resolution
	{
		IrrlichtDevice *nulldevice = createDevice(video::EDT_NULL);
		params.windowres = nulldevice->getVideoModeList()->getDesktopResolution();
		nulldevice->drop();
	}
	

	StackEditor stackEditor = StackEditor();
	//setup the stackEditor
	Helpers::mainStackEditor = &stackEditor;

	IrrlichtDevice *device = createDevice(video::EDT_DIRECT3D9, params.windowres, 32, false, false, false, &stackEditor);
	
	if (!device)
		return 1;

	Helpers::irrdevice = device;
    Log::writeToLog(Log::INFO, "Irrlicht device ok...");

	//set caption
	device->setWindowCaption(L"Orbiter Stack Editor - unnamed");

	//set the working directory
	std::string directory = device->getFileSystem()->getWorkingDirectory().c_str();
	std::replace(directory.begin(), directory.end(), '/', '\\');
	Helpers::workingDirectory = directory;

	//pass it off to StackEditor
	stackEditor.setupDevice(device, params.toolboxset);
	//and run!
	stackEditor.loop();
	device->drop();
	
	return 0;
} 