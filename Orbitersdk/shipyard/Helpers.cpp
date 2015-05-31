#include "Helpers.h"

std::string Helpers::workingDirectory = "";
Shipyard* Helpers::mainShipyard = 0;
std::mutex Helpers::videoDriverMutex;
bool Helpers::readLine(ifstream& file, std::vector<std::string>& tokens, const std::string &delimiters)
{
	std::string line;

	//return if we reached eof
	if (!getline(file, line))
		return false;
	//cut everything beyond a ';'
	if (line.find_first_of(';') != std::string::npos)
		line.erase(line.find_first_of(';'), std::string::npos);
	//remove extra spaces
	removeExtraSpaces(line);

	// Skipping delimiters at the beginning
	std::string::size_type lastPos = line.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	std::string::size_type pos = line.find_first_of(delimiters, lastPos);

	while (pos != std::string::npos || lastPos != std::string::npos)
	{
		// Found a token, add it to the vector.
		tokens.push_back(line.substr(lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = line.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = line.find_first_of(delimiters, lastPos);
	}
	return true;
}

int Helpers::stringToInt(const std::string& inputString)
{
	int num;
	std::istringstream(inputString) >> num;
	return num;
}
double Helpers::stringToDouble(const std::string& inputString)
{
	double num;
	std::istringstream(inputString) >> num;
	return num;
}

video::ITexture* Helpers::readDDS(std::string path, std::string name, video::IVideoDriver* driver)
{

	videoDriverMutex.lock();
	//get the DDS
	irrutils::DdsImage ddsImage = irrutils::DdsImage(path.c_str(), driver);
	video::IImage* image = ddsImage.getImage();
	video::ITexture* texture = driver->addTexture(name.c_str(), image);
	videoDriverMutex.unlock();
	texture->grab();
	return texture;
}

void Helpers::removeExtraSpaces(std::string& str)
{
	std::string::iterator new_end = std::unique(str.begin(), str.end(), BothAreSpaces);
	str.erase(new_end, str.end());
}

void Helpers::writeToLog(std::string &logMsg, bool clear)
{
	std::ios_base::openmode mode = ios::app;
	if (clear == true)
	{
		mode = ios::out;
	}
	std::ofstream logFile = std::ofstream("./StackEditor/StackEditor.log", mode);
	logFile << logMsg;
	logFile.close();
}

void Helpers::writeVectorToLog(const std::string& vecName, irr::core::vector3df vec)
{
	std::ofstream logFile = std::ofstream("./StackEditor/StackEditor.log", ios::app);
	logFile << vecName << "-X: " << vec.X << " Y: " << vec.Y << " Z: " << vec.Z << "\n";
	logFile.close();
}

double Helpers::min(double v1, double v2)
{
	if (v1 <= v2)
	{
		return v1;
	}
	return v2;
}

double Helpers::max(double v1, double v2)
{
	if (v1 >= v2)
	{
		return v1;
	}
	return v2;
}

