#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <mutex>
#include <algorithm>
#include <irrlicht.h>
#include <sstream>

#include "Version.h"
#include "DdsImage.h"
#include "Common.h"

//using namespace std;
using namespace irr;

class VesselSceneNode;

class Shipyard;
struct CONFIGPARAMS
{
	std::string toolboxset;
	core::dimension2d<u32> windowres;
};

class Helpers
{
public:
	static Shipyard* mainShipyard;
	static std::string workingDirectory;
	static bool readLine(ifstream& file, std::vector<std::string>& tokens, const std::string &delimiters = std::string("\t ="));
	static void tokenize(std::string line, std::vector<std::string>& tokens, const std::string &delimiters);
	static void slashreplace(std::string &str);
	static int stringToInt(const std::string& inputString);
	static double stringToDouble(const std::string& inputString);
	static video::ITexture* readDDS(std::string path, std::string name, video::IVideoDriver* driver);
	static bool BothAreSpaces(char lhs, char rhs) { return (lhs == rhs) && (lhs == ' '); }
	static void removeExtraSpaces(std::string& str);
	static void writeToLog(std::string &logMsg, bool clear = false);
	static void writeVectorToLog(const std::string& vectorName, irr::core::vector3df vec);
	static std::mutex videoDriverMutex;
//	static double min(double v1, double v2);
//	static double max(double v1, double v2);
	static std::string meshNameToImageName(std::string meshname);
	static IrrlichtDevice *irrdevice;
	static CONFIGPARAMS loadConfigParams();
	static void resetDirectory();

    static void setVesselMap(std::map<unsigned int, VesselSceneNode*>* _vesselMap);
    static void registerVessel(unsigned int uid, VesselSceneNode* vessel);
    static void unregisterVessel(unsigned int uid);
    static VesselSceneNode* getVesselByUID(unsigned int uid);
    static bool isUIDRegistered(unsigned int uid);
    static unsigned int findFreeUID(unsigned int startNum);

private:
    static std::map<unsigned int, VesselSceneNode*>* vesselMap;
};