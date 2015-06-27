#define STRICT 1
#define ORBITER_MODULE

#include "Common.h"
#include "orbitersdk.h"
#include <algorithm>
#include "Shipyard.h"
#include "StackExport.h"
#include "Helpers.h"


#ifdef _IRR_WINDOWS_
#pragma comment(linker, "/subsystem:windows")
#endif

using namespace irr;

DWORD g_dwCmd;
void startSEThread(void *context);
void OpenSE();
ExportData orbiterexport;
StackExport *exporter = NULL;

DLLCLBK void InitModule(HINSTANCE hDLL)
{

	// To allow the user to open our new dialog box, we create
	// an entry in the "Custom Functions" list which is accessed
	// in Orbiter via Ctrl-F4.
	g_dwCmd = oapiRegisterCustomCmd("StackEditor",
		"Opens StackEditor in an external window",
		startSEThread, NULL);

}

DLLCLBK void ExitModule(HINSTANCE hDLL)
{
	oapiUnregisterCustomCmd(g_dwCmd);
}

DLLCLBK void opcPostStep(double  simt, double  simdt, double  mjd)
{
	//check if an export has been ordered
	if (!orbiterexport.locked && orbiterexport.exporting)
	{
		if (!exporter)
		{
			//process data and create vessels in orbiter
			exporter = new StackExport(&orbiterexport);
		}
		//create and dock a new vessel every frame. Bulk creation and docking works only most of the time in my expierience.
		if (exporter->advanceQueue())
		{
			//the process has finished, clear the data
			orbiterexport.exporting = false;
			orbiterexport.name = "";
			orbiterexport.stack = NULL;
			delete exporter;
			exporter = NULL;
		}
	}
}


void startSEThread(void *context)
{
	std::thread seThread(OpenSE);
	seThread.detach();
}

void OpenSE()
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


	Shipyard shipyard = Shipyard(&orbiterexport);
	//setup the shipyard
	Helpers::mainShipyard = &shipyard;

	IrrlichtDevice *device = createDevice(video::EDT_DIRECT3D9, params.windowres, 32, false, false, false, &shipyard);

	if (!device)
		return;

	Helpers::irrdevice = device;
	Helpers::writeToLog(std::string("\n Irrlicht device ok..."));

	//set caption
	device->setWindowCaption(L"Orbiter Shipyard - unnamed");

	//set the working directory
	std::string directory = device->getFileSystem()->getWorkingDirectory().c_str();
	std::replace(directory.begin(), directory.end(), '/', '\\');
	Helpers::workingDirectory = directory;

	//pass it off to Shipyard
	shipyard.setupDevice(device, params.toolboxset);
	//and run!
	shipyard.loop();
	device->drop();

	return;
} 