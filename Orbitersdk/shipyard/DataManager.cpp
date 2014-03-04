#include "DataManager.h"



DataManager::DataManager()
{

}

DataManager::~DataManager()
{

}


OrbiterMesh* DataManager::GetGlobalMesh(string meshName, video::IVideoDriver* driver)
//returns pointer to the requsted mesh. Loads mesh if it doesn't exist yet. returns NULL if mesh could not be created
{
	map<string, OrbiterMesh*>::iterator pos = meshMap.find(meshName);
	if (pos == meshMap.end()) 
	//meshName not found in the map, load mesh from file
	{
		OrbiterMesh *newMesh = new OrbiterMesh;
		if (newMesh->setupMesh(string(Helpers::workingDirectory + "\\Meshes\\" + meshName + ".msh"), driver))
		//mesh loaded succesfully, enter in map and return pointer
		{
			meshMap[meshName] = newMesh;
			return newMesh;
		}
		else
		//mesh not found, delete allocated pointer and return NULL
		{
			delete newMesh;
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
	map<string, VesselData*>::iterator pos = cfgMap.find(cfgName);
	if (pos == cfgMap.end())
		//cfg not found in the map, load from file
	{
		VesselData *newVessel = LoadVesselData(cfgName, driver);
		if (newVessel != NULL)
			//cfg loaded succesfully, enter in map and return pointer
		{
			cfgMap[cfgName] = newVessel;
		}
		return newVessel;
	}
	else
		//cfg found in map, return pointer
	{
		return pos->second;
	}
}


video::ITexture *DataManager::GetGlobalImg(string imgName, video::IVideoDriver* driver)
//returns pointer to an image, loads it from file if image is requested for the first time
{
	map<string, video::ITexture*>::iterator pos = imgMap.find(imgName);
	if (pos == imgMap.end())
	//image Name not found in the map, load mesh from file
	{
	
		string completeImgPath = Helpers::workingDirectory + "\\" + imgName;
		IImage *img = driver->createImageFromFile(completeImgPath.data());
		
		if (img != NULL)
		//image loaded succesfully, enter in map and return pointer
		{
			video::ITexture *newTex = driver->addTexture("tbxtex", img);
			imgMap[imgName] = newTex;
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
		if (readingDockingPorts && tokens.size() == 9)
			newVessel->dockingPorts.push_back(OrbiterDockingPort(
			core::vector3d<f32>(Helpers::stringToDouble(tokens[0]),
			Helpers::stringToDouble(tokens[1]), Helpers::stringToDouble(tokens[2])),
			core::vector3d<f32>(Helpers::stringToDouble(tokens[3]),
			Helpers::stringToDouble(tokens[4]), Helpers::stringToDouble(tokens[5])),
			core::vector3d<f32>(Helpers::stringToDouble(tokens[6]),
			Helpers::stringToDouble(tokens[7]), Helpers::stringToDouble(tokens[8]))));
		//now see if this is the beginning of a docking port list
		if (tokens[0].compare("BEGIN_DOCKLIST") == 0)
		{
			readingDockingPorts = true;
		}


		//now see if it is a MeshName
		//put it in lowercase to start
		transform(tokens[0].begin(), tokens[0].end(), tokens[0].begin(), ::tolower);
		//see if it matches
		if (tokens[0].compare("meshname") == 0)
			//load the mesh!
		{
			newVessel->vesselMesh = GetGlobalMesh(tokens[1], driver);
		}


		if (tokens[0].compare("imagebmp") == 0)
		//check for scened image file
		{
			newVessel->vesselImg = GetGlobalImg(tokens[1], driver);
		}

		//clear tokens
		tokens.clear();
	}
	return newVessel;
}
