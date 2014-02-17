#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <irrlicht.h>

#include "DdsImage.h"

using namespace std;
using namespace irr;

class Helpers
{
public:
	static bool readLine(ifstream& file, vector<std::string>& tokens);
	static int stringToInt(const std::string& inputString);
	static double stringToDouble(const std::string& inputString);
	static video::ITexture* readDDS(std::string path, std::string name, video::IVideoDriver* driver);
};