//Copyright (c) 2015 Christopher Johnstone(meson800) and Benedict Haefeli(jedidia)
//The MIT License - See ../../LICENSE for more info
//this class handles creation of a vessel stack in Orbiter
//should only be included in main_orbiter.cpp

#pragma once


class StackExport
{
public:
	StackExport(ExportData *data);
	~StackExport();
	bool advanceQueue();								//creates the next vessel in orbiter

private:
	void prepareExportData();							//turns the data from the stack in _data to something more easily processable

	std::queue<VesselExport> _vessels;
	std::vector<OBJHANDLE> _createdvessels;
	UINT vessels_processed;
	ExportData *_data;
};

