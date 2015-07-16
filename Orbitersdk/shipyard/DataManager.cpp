#include "SE_PhotoStudio.h"
#include "DataManager.h"



DataManager::DataManager()
{
	_runningthreads = 0;
}

DataManager::~DataManager()
{
	//make sure the loading threads are done before deallocating
	while (_runningthreads > 0)	{ }
	
	for (std::map<std::string, OrbiterMesh*>::iterator pos = meshMap.begin(); pos != meshMap.end(); ++pos)
	{
		delete pos->second;
	}
	meshMap.clear();

	for (std::map<std::string, VesselData*>::iterator pos = cfgMap.begin(); pos != cfgMap.end(); ++pos)
	{
		delete pos->second;
	}
	cfgMap.clear();

	for (std::map<std::string, ToolboxData*>::iterator pos = toolboxMap.begin(); pos != toolboxMap.end(); ++pos)
	{
		delete pos->second;
	}
	cfgMap.clear();

	imgMap.clear();			//Irrlicht will drop the textures itself
	delete photostudio;
}

void DataManager::Initialise(IrrlichtDevice *device)
{
	//set up the photostudio for when we have to take pictures of meshes
	photostudio = new SE_PhotoStudio(device);
}

OrbiterMesh* DataManager::GetGlobalMesh(string meshName, video::IVideoDriver* driver)
//returns pointer to the requsted mesh. Loads mesh if it doesn't exist yet. returns NULL if mesh could not be created
{
	//prevent race condition
	meshMutex.lock();
	map<string, OrbiterMesh*>::iterator pos = meshMap.find(meshName);
	bool temp = (pos == meshMap.end());	//split this check here so we can unlock the mutex ASAP
	meshMutex.unlock();

	if (temp) 
	//meshName not found in the map, load mesh from file
	{
		OrbiterMesh *newMesh = new OrbiterMesh;
		if (newMesh->setupMesh(string(Helpers::workingDirectory + "\\Meshes\\" + meshName + ".msh"), driver))
		//mesh loaded succesfully, enter in map and return pointer
		{
			//lock to prevent race condition
			meshMutex.lock();
			meshMap[meshName] = newMesh;
			meshMutex.unlock();

			return newMesh;
		}
		else
		//mesh not found, delete allocated pointer and return NULL
		{
			delete newMesh;
			Log::writeToLog(std::string("Could not load mesh: " + meshName + ".msh"), Log::ERR);
			return NULL;
		}
	}
	else 
	//mesh found in map, return pointer
	{
		return pos->second;
	}
}


VesselData* DataManager::GetGlobalConfig(string cfgName, video::IVideoDriver* driver)
//returns pointer to the requsted VesselData. Loads VesselData if it doesn't exist yet. returns NULL if cfg could not be found
{
	configMutex.lock();
	//insure a consistent style for the key
	transform(cfgName.begin(), cfgName.end(), cfgName.begin(), ::tolower);
	Helpers::slashreplace(cfgName);
	map<string, VesselData*>::iterator pos = cfgMap.find(cfgName);
	bool temp = (pos == cfgMap.end());	//again, put this here so we can unlock ASAP
	configMutex.unlock();

	if (temp)
	//cfg not found in the map, load from file
	{
		VesselData *newVessel = LoadVesselData(cfgName, driver);
		if (newVessel != NULL)
		//cfg loaded succesfully, enter in map and return pointer
		{
			configMutex.lock();
			cfgMap[cfgName] = newVessel;
			configMutex.unlock();
			//Helpers::writeToLog(std::string("\n Loaded vessel config:" + cfgName));
		}
		else
		{
			Log::writeToLog(std::string("Could not load cfg: " + cfgName), Log::ERR);
		}
		_runningthreads--;		//the loading thread will terminate after returning
		return newVessel;
	}
	else
	//cfg found in map, return pointer
	{
		return pos->second;
	}
}

ToolboxData* DataManager::GetGlobalToolboxData(std::string configName, video::IVideoDriver* driver)
//returns pointer to requested ToolboxData
{
	toolboxMutex.lock();
	map<string, ToolboxData*>::iterator pos = toolboxMap.find(configName);
	bool temp = (pos == toolboxMap.end());	//again, put this here so we can unlock ASAP
	toolboxMutex.unlock();

	if (temp)
	//data not found in the map, load from file
	{
		//create new toolbox data
		ToolboxData* toolboxData = new ToolboxData;
		//set the config file path
		toolboxData->configFileName = configName;

		//get ready to read file
		vector<string> tokens;
		string completeCfgPath = Helpers::workingDirectory + "\\config\\vessels\\" + configName;
		ifstream configFile = ifstream(completeCfgPath.c_str());
		if (!configFile) return NULL;

		//these will be needed if the vessel is an ims module, otherwise it will have to parse through the whole file again
		string maxfuel("");						
		string mass("");
		bool iscommandmodule = false;

		//now look for the image. its name will be derived from the meshname. if the vessel is an IMS module, load the ims properties
		//note: this could have been structured a lot better by simply parsing through the whole file a second time in search for ims parameters.
		//I decided to sacrifice some structure and readability to gain a bit of loading speed.
		while (Helpers::readLine(configFile, tokens))
		{
			//check to see if there are any tokens
			if (tokens.size() == 0)
				continue;

			//put it in lowercase to start
			transform(tokens[0].begin(), tokens[0].end(), tokens[0].begin(), ::tolower);

			if (tokens[0].compare("module") == 0 && tokens.size() >= 2)
			//potentially a command module. They don't have a module type defined, so we can't rely on that for identification later on
			{
				transform(tokens[1].begin(), tokens[1].end(), tokens[1].begin(), ::tolower);
				if (tokens[1].compare("ims/ims") == 0 || tokens[1].compare("ims\\ims") == 0)
				//it is a command module
				{
					//if the image has already been found, immediately pass on the file
					if (toolboxData->toolboxImage != NULL)
					{
						toolboxData->imsData = new ImsData(Helpers::irrdevice->getGUIEnvironment(), "Command", configFile, mass, maxfuel);
					}
					//if not, mark it so it can be passed on as soon as we have the image
					else
					{
						iscommandmodule = true;
					}
				}
			}
			if (tokens[0].compare("mass") == 0 && tokens.size() >= 2)
			{
				mass = tokens[1];
			}
			else if (tokens[0].compare("maxfuel") == 0 && tokens.size() >= 2)
			{
				maxfuel = tokens[1];
			}
			else if (tokens[0].compare("meshname") == 0 && tokens.size() >= 2)
			//check for image file
			{
				std::string imgname = Helpers::meshNameToImageName(tokens[1]);
				toolboxData->toolboxImage = GetGlobalImg(imgname, configName, driver);
				if (toolboxData->toolboxImage == NULL)
				{
					break;			//this module is invalid, don't waste any time
				}
				//the module has been recognised as a command module before the image was declared. load ims properties now.
				if (iscommandmodule)
				{
					toolboxData->imsData = new ImsData(Helpers::irrdevice->getGUIEnvironment(), "Command", configFile, mass, maxfuel);
				}
			}
			else if (tokens[0].compare("moduletype") == 0 && tokens.size() >= 2)
			//we got ourselves an ims module. create a data instance and pass the file to it to load the parameters
			//technically it would be possible for the module type to be declared before the meshname, but I've never seen it done, so I'm taking the gamble
			{
				toolboxData->imsData = new ImsData(Helpers::irrdevice->getGUIEnvironment(), tokens[1], configFile, mass, maxfuel);
			}
			tokens.clear();
		}

		configFile.close();

		if (toolboxData->toolboxImage != NULL)
		//data loaded succesfully, background load data, enter in map and return pointer
		{
			std::thread backgroundLoadThread = std::thread(&DataManager::GetGlobalConfig, this, configName, driver);
			_runningthreads++;
			//detach the thread to continue background loading
			backgroundLoadThread.detach();

			toolboxMutex.lock();
			toolboxMap[configName] = toolboxData;
			toolboxMutex.unlock();
			//Helpers::writeToLog(std::string("\n Loaded toolbox data:" + configName));
		}
		else
		{
			Log::writeToLog(std::string("Could not load cfg while loading toolbox data: " + configName), Log::ERR);
			delete toolboxData;
			toolboxData = NULL;
		}
		return toolboxData;
	}
	else
		//data found in map, return pointer
	{
		return pos->second;
	}
}


video::ITexture *DataManager::GetGlobalImg(string imgname, string configname, video::IVideoDriver* driver)
//returns pointer to an image, loads it from file if image is requested for the first time
{
	imgMutex.lock();
	map<string, video::ITexture*>::iterator pos = imgMap.find(imgname);
	bool temp = (pos == imgMap.end());	//put check hear so we can unlock the mutex ASAP
	imgMutex.unlock();

	if (temp)
	//image Name not found in the map, load mesh from file
	{
	
		string completeImgPath = Helpers::workingDirectory + "\\StackEditor\\Images\\" + imgname;
		Helpers::videoDriverMutex.lock();
		IImage *img = driver->createImageFromFile(completeImgPath.data());
		Helpers::videoDriverMutex.unlock();
		
		ITexture *newTex = NULL;

		if (img != NULL)
		//image loaded succesfully, enter in map and return pointer
		{
			Helpers::videoDriverMutex.lock();
			newTex = driver->addTexture("tbxtex", img);
			img->drop();
			Helpers::videoDriverMutex.unlock();
		}
		else
		//image doesn't exist, need to create it
		{
			VesselData *data = GetGlobalConfig(configname, driver);
			if (data)
			{
				newTex = photostudio->makePicture(data, imgname);
			}
		}

		if (newTex != NULL)
		//register the texture in the data manager for future retrieval and return it
		{
			imgMutex.lock();
			imgMap[imgname] = newTex;
			imgMutex.unlock();
			return newTex;
		}
		else
		//something went wrong, dump to log
		{
			Log::writeToLog("Unable to find or create image: " + imgname, Log::ERR);
			return NULL;
		}
	}
	else
	//image found in map, return pointer
	{
		return pos->second;
	}

}

VesselData *DataManager::LoadVesselData(string configFileName, video::IVideoDriver* driver)
//loads vessel data from config file. returns NULL if file not found.
{
	bool meshDefined = false;
	bool portsDefined = false;

	vector<string> tokens;
	string completeCfgPath = Helpers::workingDirectory + "\\config\\vessels\\" + configFileName;
	ifstream configFile = ifstream(completeCfgPath.c_str());
	if (!configFile) return NULL;

	VesselData *newVessel = new VesselData;
	newVessel->className = configFileName;

	bool readingDockingPorts = false;

	while (Helpers::readLine(configFile, tokens))
	{
		//check to see if there are any tokens
		if (tokens.size() == 0)
			continue;

		//or if it is the end
		if (tokens[0].compare("END_DOCKLIST") == 0)
		{
			readingDockingPorts = false;
		}
		//if we are reading docking ports, create a new docking port!
		if (readingDockingPorts && tokens.size() >= 9)
		{
			newVessel->dockingPorts.push_back(OrbiterDockingPort(
				core::vector3d<f32>(Helpers::stringToDouble(tokens[0]),
				Helpers::stringToDouble(tokens[1]), Helpers::stringToDouble(tokens[2])),
				core::vector3d<f32>(Helpers::stringToDouble(tokens[3]),
				Helpers::stringToDouble(tokens[4]), Helpers::stringToDouble(tokens[5])),
				core::vector3d<f32>(Helpers::stringToDouble(tokens[6]),
				Helpers::stringToDouble(tokens[7]), Helpers::stringToDouble(tokens[8]))));
			newVessel->dockingPorts[newVessel->dockingPorts.size() - 1].index = newVessel->dockingPorts.size() - 1;
			if (tokens.size() > 9)
			{
				Log::writeToLog(std::string("Unusual docking port definition in cfg file " + configFileName) + ": definition contains more than 9 entries!",Log::WARN);
			}
		}
		else if (readingDockingPorts && tokens.size() < 9)
		{
			Log::writeToLog(std::string("Invalid docking port definition in cfg file " + configFileName) + ": definition contains less than 9 entries!", Log::ERR);
		}
		//now see if this is the beginning of a docking port list
		if (tokens[0].compare("BEGIN_DOCKLIST") == 0)
		{
			readingDockingPorts = true;
			portsDefined = true;
		}


		//now see if it is a MeshName
		//put it in lowercase to start
		transform(tokens[0].begin(), tokens[0].end(), tokens[0].begin(), ::tolower);
		//see if it matches
		if (tokens[0].compare("meshname") == 0)
			//load the mesh!
		{
			newVessel->vesselMesh = GetGlobalMesh(tokens[1], driver);
			if (newVessel->vesselMesh != NULL)
			{
				meshDefined = true;
			}
		}

		//clear tokens
		tokens.clear();
	}
	configFile.close();

	if (!meshDefined)
	{
		Log::writeToLog(std::string("No mesh defined in " + configFileName), Log::WARN);
		delete newVessel;
		newVessel = NULL;
	}
	if (newVessel && !portsDefined)
	{
		Log::writeToLog(std::string("No docking ports defined in " + configFileName), Log::WARN);
		delete newVessel;
		newVessel = NULL;
	}
	return newVessel;
}
