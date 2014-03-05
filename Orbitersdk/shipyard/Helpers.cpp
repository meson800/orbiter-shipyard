#include "Helpers.h"

std::string Helpers::workingDirectory = "";
bool Helpers::readLine(ifstream& file, std::vector<std::string>& tokens, const string &delimiters)
{
	std::string line;

	//return if we reached eof
	if (!getline(file, line))
		return false;
	//cut everything beyond a ';'
	if (line.find_first_of(';') != string::npos)
		line.erase(line.find_first_of(';'), string::npos);
	//remove extra spaces
	removeExtraSpaces(line);

	// Skipping delimiters at the beginning
	string::size_type lastPos = line.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	string::size_type pos = line.find_first_of(delimiters, lastPos);

	while (pos != string::npos || lastPos != string::npos) 
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

int Helpers::stringToInt(const string& inputString)
{
	int num;
	istringstream(inputString) >> num;
	return num;
}
double Helpers::stringToDouble(const string& inputString)
{
	double num;
	istringstream(inputString) >> num;
	return num;
}

video::ITexture* Helpers::readDDS(string path, string name, video::IVideoDriver* driver) 
{

	//get the DDS
	irrutils::DdsImage ddsImage = irrutils::DdsImage(path.c_str(), driver);
	video::IImage* image = ddsImage.getImage();
	video::ITexture* texture = driver->addTexture(name.c_str(), image);
	texture->grab();
	return texture;
}

void Helpers::removeExtraSpaces(std::string& str)
{
	std::string::iterator new_end = std::unique(str.begin(), str.end(), BothAreSpaces);
	str.erase(new_end, str.end());
}

void Helpers::writeToLog(std::string &logMsg, bool close)
{
	static ofstream logFile = ofstream("./StackEditor/StackEditor.log", ios::out);
	logFile << logMsg;

	if (close)
	{
		logFile << "end log";
		logFile.close();
	}
}