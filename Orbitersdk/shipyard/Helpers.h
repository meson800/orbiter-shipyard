#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <mutex>
#include <algorithm>
#include <irrlicht.h>

#include "DdsImage.h"

//using namespace std;
//using namespace irr;

class Shipyard;

class Helpers
{
public:
	static Shipyard* mainShipyard;
	static std::string workingDirectory;
	static bool readLine(ifstream& file, std::vector<std::string>& tokens, const std::string &delimiters = std::string("\t ="));
	static int stringToInt(const std::string& inputString);
	static double stringToDouble(const std::string& inputString);
	static video::ITexture* readDDS(std::string path, std::string name, video::IVideoDriver* driver);
	static bool BothAreSpaces(char lhs, char rhs) { return (lhs == rhs) && (lhs == ' '); }
	static void removeExtraSpaces(std::string& str);
	static void writeToLog(std::string &logMsg, bool clear = false);
	static std::mutex videoDriverMutex;
	static double min(double v1, double v2);
	static double max(double v1, double v2);
};