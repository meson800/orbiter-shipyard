#include "Helpers.h"

bool Helpers::readLine(ifstream& file, std::vector<std::string>& tokens)
{
	std::string line;

	//return if we reached eof
	if (getline(file, line))
		return false;

	while (line.length() > 0)
	{
		//push the first token
		tokens.push_back(line.substr(0, line.find_first_of(' ')));
		//remove the first token
		line.erase(0, line.find_first_of(' ') + 1);
	}
	return true;
}