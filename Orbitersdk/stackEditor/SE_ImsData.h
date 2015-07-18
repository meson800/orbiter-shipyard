#pragma once
#include "Common.h"

//class to describe ims properties

class ImsData
{
public:
	ImsData(gui::IGUIEnvironment *env, std::string moduletype, ifstream &configfile, std::string mass, std::string maxfuel);
	~ImsData();
	void setVisible(bool visible);

private:
//	vector<pair<std::string, std::string>>props_;				//contains all the properties

	void loadImsProperties(ifstream &configfile, std::string mass, std::string maxfuel, string moduletype);
	std::string convertUnitString(std::string u, std::string unit);
	void addRow(std::string first, std::string second);
	gui::IGUITable *data;
	UINT rowindex;
};

