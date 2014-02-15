#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

class Helpers
{
public:
	static bool readLine(ifstream& file, vector<string>& tokens);
};