#include "DataManager.h"



DataManager::DataManager()
{

}

DataManager::~DataManager()
{
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

	imgMap.clear();			//Irrlicht will drop the textures itself
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
			Helpers::writeToLog(std::string("\n ERROR: could not load mesh: " + meshName + ".msh"));
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
			Helpers::writeToLog(std::string("\n Loaded vessel config:" + cfgName));
		}
		else
		{
			Helpers::writeToLog(std::string("\n ERROR: could not load cfg: " + cfgName));
		}
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
		//now just look for the imagebmp

		while (Helpers::readLine(configFile, tokens))
		{
			//check to see if there are any tokens
			if (tokens.size() == 0)
				continue;

			//put it in lowercase to start
			transform(tokens[0].begin(), tokens[0].end(), tokens[0].begin(), ::tolower);

			if (tokens[0].compare("imagebmp") == 0 && tokens.size() >= 2)
				//check for scened image file
			{
				toolboxData->toolboxImage = GetGlobalImg(tokens[1], driver);
			}
			tokens.clear();
		}

		if (toolboxData->toolboxImage != NULL)
			//data loaded succesfully, background load data, enter in map and return pointer
		{
			std::thread backgroundLoadThread = std::thread(&DataManager::GetGlobalConfig, this, configName, driver);
			//detach the thread to continue background loading
			backgroundLoadThread.detach();

			toolboxMutex.lock();
			toolboxMap[configName] = toolboxData;
			toolboxMutex.unlock();
			Helpers::writeToLog(std::string("\n Loaded toolbox data:" + configName));
		}
		else
		{
			Helpers::writeToLog(std::string("\n ERROR: could not load cfg while loading toolbox data: " + configName));
		}
		return toolboxData;
	}
	else
		//data found in map, return pointer
	{
		return pos->second;
	}
}


video::ITexture *DataManager::GetGlobalImg(string imgName, video::IVideoDriver* driver)
//returns pointer to an image, loads it from file if image is requested for the first time
{
	imgMutex.lock();
	map<string, video::ITexture*>::iterator pos = imgMap.find(imgName);
	bool temp = (pos == imgMap.end());	//put check hear so we can unlock the mutex ASAP
	imgMutex.unlock();

	if (temp)
	//image Name not found in the map, load mesh from file
	{
	
		string completeImgPath = Helpers::workingDirectory + "\\" + imgName;
		Helpers::videoDriverMutex.lock();
		IImage *img = driver->createImageFromFile(completeImgPath.data());
		Helpers::videoDriverMutex.unlock();
		
		if (img != NULL)
		//image loaded succesfully, enter in map and return pointer
		{
			Helpers::videoDriverMutex.lock();
			video::ITexture *newTex = driver->addTexture("tbxtex", img);
			Helpers::videoDriverMutex.unlock();

			imgMutex.lock();
			imgMap[imgName] = newTex;
			img->drop();
			imgMutex.unlock();

			return newTex;
		}
		else
		//image not found
		{
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
			if (tokens.size() > 9)
			{
				Helpers::writeToLog(std::string("\n WARNING: Unusual docking port definition in cfg file " + configFileName) + ": definition contains more than 9 entries!");
			}
		}
		else if (readingDockingPorts && tokens.size() < 9)
		{
			Helpers::writeToLog(std::string("\n ERROR: Invalid docking port definition in cfg file " + configFileName) + ": definition contains less than 9 entries!");
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
			meshDefined = true;
		}


		if (tokens[0].compare("imagebmp") == 0)
		//check for scened image file
		{
			newVessel->vesselImg = GetGlobalImg(tokens[1], driver);
		}

		//clear tokens
		tokens.clear();
	}

	if (!meshDefined)
	{
		Helpers::writeToLog(std::string("\n WARNING: no mesh defined in " + configFileName));
	}
	if (!portsDefined)
	{
		Helpers::writeToLog(std::string("\n WARNING: no docking ports defined in " + configFileName));
	}
	return newVessel;
}
