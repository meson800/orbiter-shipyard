#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

class Helpers
{
public:
	static bool readLine(ifstream& file, vector<string>& tokens);
	static int stringToInt(const string& inputString)
};