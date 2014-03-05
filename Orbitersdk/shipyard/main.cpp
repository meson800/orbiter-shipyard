#include "Common.h"
#include <algorithm>
#include "Shipyard.h"
#include "Helpers.h"

#ifdef _IRR_WINDOWS_
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

using namespace irr;



int main()
{

	//loading configuration from StackEditor.cfg
	core::dimension2d<u32> windowRes = dimension2d<u32>(0,0);
	std::string tbxSet("default");
	vector<std::string> tokens;
	std::string cfgPath = std::string(Helpers::workingDirectory + "\\StackEditor\\StackEditor.cfg");
	ifstream configFile = ifstream(cfgPath.c_str());
	if (configFile)
	{
		while (Helpers::readLine(configFile, tokens))
		{
			if (tokens.size() == 0) continue;

			if (tokens[0].compare("resolution") == 0 && tokens.size() == 3)
			{
				windowRes = dimension2d<u32>(UINT(Helpers::stringToInt(tokens[1])), UINT(Helpers::stringToInt(tokens[2])));
			}

			if (tokens[0].compare("tbxset") == 0) 
			{
				if (tokens.size() > 2)
				//putting together the rest of the string again if the folder name contains a space
				for (UINT i = 1; i < tokens.size(); ++i)
				{
					tbxSet += std::string(tokens[i] + " ");
				}
				else
				{
					tbxSet = tokens[1];
				}
			}
		}
	}

	if (windowRes == dimension2d<u32>(0, 0))
	//resolution not specified, create a NULL device to detect screen resolution
	{
		IrrlichtDevice *nulldevice = createDevice(video::EDT_NULL);
		windowRes = nulldevice->getVideoModeList()->getDesktopResolution();
		nulldevice->drop();
	}
	

	Shipyard shipyard = Shipyard();

	IrrlichtDevice *device = createDevice(video::EDT_DIRECT3D9, windowRes, 32, false, false, false, &shipyard);
	if (!device)
		return 1;

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
	
	return 0;
} 