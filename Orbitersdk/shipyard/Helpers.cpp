#include "Helpers.h"

bool Helpers::readLine(ifstream& file, std::vector<std::string>& tokens)
{
	std::string line;

	//return if we reached eof
	if (getline(file, line))
		return false;
	//cut everything beyond a ';'
	line.substr(line.find_first_of(';'), string::npos);

	while (line.length() > 0)
	{
		//push the first token
		tokens.push_back(line.substr(0, line.find_first_of(' ')));
		//remove the first token
		line.erase(0, line.find_first_of(' ') + 1);
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