//Copyright (c) 2015 Christopher Johnstone(meson800) and Benedict Haefeli(jedidia)
//The MIT License - See ../../LICENSE for more info
#include "SE_ImsData.h"


ImsData::ImsData(gui::IGUIEnvironment *env, std::string moduletype, ifstream &configfile, string mass, string maxfuel)
{
	IVideoDriver *driver = Helpers::irrdevice->getVideoDriver();
	rowindex = 0;

	//set a preliminary position. We'll have to change the height after loading, but if it isn't high enough to begin with, it will constantly display a vertical scrollbar
	core::rect<s32> pos = core::rect<s32>(driver->getScreenSize().Width - 400, 0,
		driver->getScreenSize().Width, driver->getScreenSize().Height - 130);
	data = env->addTable(pos, env->getRootGUIElement());
	data->setVisible(false);
	
	//populate the table with the properties from the file
	data->addColumn(L"moduletype");
	data->addColumn(std::wstring(moduletype.begin(), moduletype.end()).c_str());
	data->setColumnWidth(0, pos.getWidth() / 2 - 1);
	data->setColumnWidth(1, pos.getWidth() / 2 - 1);
	if (mass.compare("") != 0)
	//the mass has already been parsed before the file got here. write it to the table
	{
		addRow("dry mass", convertUnitString(mass, "kg"));

		if (maxfuel.compare("") != 0)
		//maxfuel has already been parsed before the file got here. write it to the table
		{
			double fullmass = Helpers::stringToDouble(mass) + Helpers::stringToDouble(maxfuel);
			addRow("wet mass", convertUnitString(to_string(fullmass), "kg"));
		}

	}
	loadImsProperties(configfile, mass, maxfuel, moduletype);

	//calculate height now that the number of rows is known
	//int bugme = env->getSkin()->getFont()->getKerningHeight();
	pos.UpperLeftCorner.Y = (irr::s32)(driver->getScreenSize().Height - 130 - (data->getRowCount() + 1) * 
		(float)(env->getSkin()->getFont()->getDimension(L"J").Height) * 1.3);		//this is a rough approximation for the row height. Irrlicht doesn't provide a way to retrieve it.
	data->setRelativePosition(pos);
}


ImsData::~ImsData()
{
}

void ImsData::setVisible(bool visible)
{
	data->setVisible(visible);
}

void ImsData::addRow(std::string first, std::string second)
{
	data->addRow(rowindex);
	data->setCellText(rowindex, 0, first.c_str());
	data->setCellText(rowindex, 1, second.c_str());
	rowindex++;
}



void ImsData::loadImsProperties(ifstream &configfile, string mass, string maxfuel, string moduletype)
{
	vector<string> tokens;

	while (Helpers::readLine(configfile, tokens))
	{
		if (tokens.size() < 2)
		{
			tokens.clear();
			continue;
		}
			

		transform(tokens[0].begin(), tokens[0].end(), tokens[0].begin(), ::tolower);

		if (tokens[0].compare("mass") == 0)
		{
			mass = tokens[1];
			addRow("dry mass", convertUnitString(tokens[1], "kg"));
		}
		else if (tokens[0].compare("maxfuel") == 0)
		{
			maxfuel = tokens[1];
			double fullmass = Helpers::stringToDouble(mass) + Helpers::stringToDouble(maxfuel);
			addRow("wet mass", convertUnitString(to_string(fullmass), "kg"));
		}
		if (tokens[0].compare("optimaltemperature") == 0)
		{
			addRow("operating temperature", convertUnitString(tokens[1], "K"));
		}
		//storage properties
		else if (tokens[0].compare("coolingcapacity") == 0 && moduletype.compare("Lifesupport") == 0)
		{
			//cooling capacity only relevant for life support modules
			addRow("cooling capacity", convertUnitString(tokens[1], "W"));
		}
		else if (tokens[0].compare("food") == 0 || tokens[0].compare("water") == 0 || tokens[0].compare("oxygen") == 0)
		{
			if (moduletype.compare("Lifesupport") == 0)
			{
				addRow(tokens[0] + " production", "for " + tokens[1] + " crew");
			}
			else
			{
				addRow(tokens[0] + " storage", convertUnitString(tokens[1], "kg"));
			}
		}
		else if (tokens[0].compare("fueltype") == 0)
		{
			addRow("propellant", tokens[1]);
			addRow("propellant mass", convertUnitString(maxfuel,"kg"));
		}
		else if (tokens[0].compare("capacity") == 0 || tokens[0].compare("crewnumber") == 0)
		{
			addRow("crew capacity", tokens[1]);
		}

		//power properties
		else if (tokens[0].compare("powerinput") == 0)
		{
			addRow("power consumption", convertUnitString(tokens[1], "W"));
		}
		else if (tokens[0].compare("poweroutput") == 0)
		{
			addRow("power generation", convertUnitString(tokens[1], "W"));
		}
		else if (tokens[0].compare("batterycapacity") == 0)
		{
			addRow("battery capacity", convertUnitString(tokens[1], "W"));
		}
		else if (tokens[0].compare("efficiency") == 0)
		{
			//just sending this off to convertUnitString to get rid of too many places behind the decimal
			string eff = convertUnitString(tokens[1], "%");
			if (eff.compare("") != 0)
			addRow("efficiency", eff);
		}
		else if (tokens[0].compare("generatorefficiency") == 0)
		{
			addRow("generator efficiency", convertUnitString(tokens[1], "%"));
		}
		//thruster properties
		else if (tokens[0].compare("thrust") == 0)
		{
			addRow("thrust", convertUnitString(tokens[1], "N"));
		}
		else if (tokens[0].compare("isp") == 0)
		{
			addRow("isp", convertUnitString(tokens[1], "m/s"));
		}
		else if (tokens[0].compare("type") == 0)
		{
			addRow("engine type", tokens[1]);
		}
		else if (tokens[0].compare("solararrayarea") == 0 || tokens[0].compare("radiatorarea") == 0)
		{
			addRow("area", tokens[1] + " m^2");
		}
		tokens.clear();
	}
}


string ImsData::convertUnitString(string u, string unit)
//takes a string with a unit and returns a string apropriately converted to kilo, mega or giga-whatevers
{
	//mass units get special treatment, as they don't follow the same k-M-G convention of other units
	bool ismass = unit.compare("kg") == 0;

	if (unit.compare("%") == 0)
	{
		if (Helpers::stringToDouble(u) > 99)
		//such high efficiencies are not really worth mentioning in the data
		{
			return "";
		}
	}
	double number = Helpers::stringToDouble(u);
	if (number >= 1e9)			//never seen it, but hell, you never know...
	{
		number /= 1e9;
		if (ismass)
		{
			unit = "Mt";
		}
		else
		{
			unit = "G" + unit;
		}
	}
	else if (number >= 1e6)		
	{
		number /= 1e6;
		if (ismass)
		{
			unit = "kt";
		}
		else
		{
			unit = "M" + unit;
		}
	}
	else if (number >= 1000)
	{
		number /= 1000;
		if (ismass)
		{
			unit = "t";
		}
		else
		{
			unit = " k" + unit;
		}
	}
	//cut to two places behind point
	string stringnumber = to_string(number).substr(0, (int)log10(number) + 4);
	return stringnumber + " " + unit;
}



/*void ImsData::fillTable(gui::IGUITable *t)
//sets the passed table up to display this module's properties
{
t->clear();
t->addColumn(L"");
t->addColumn(L"");
int columnwidth = t->getAbsoluteClippingRect().getWidth() / 2 - 1;
t->setColumnWidth(0, columnwidth);
t->setColumnWidth(1, columnwidth);

for (UINT i = 0; i < props_.size(); ++i)
{
t->addRow(i);
string bugme = props_[i].first.c_str();
string bugme2 = props_[i].second.c_str();
t->setCellText(i, 0, props_[i].first.c_str());
t->setCellText(i, 1, props_[i].second.c_str());
}
}*/
