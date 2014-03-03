#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <irrlicht.h>

#include "DdsImage.h"

using namespace std;
using namespace irr;

class Helpers
{
public:
	static std::string workingDirectory;
	static bool readLine(ifstream& file, vector<std::string>& tokens, const string &delimiters = " =");
	static int stringToInt(const std::string& inputString);
	static double stringToDouble(const std::string& inputString);
	static video::ITexture* readDDS(std::string path, std::string name, video::IVideoDriver* driver);
	static bool BothAreSpaces(char lhs, char rhs) { return (lhs == rhs) && (lhs == ' '); }
	static void removeExtraSpaces(std::string& str);
};