//Copyright (c) 2015 Christopher Johnstone(meson800) and Benedict Haefeli(jedidia)
//The MIT License - See ../../LICENSE for more info
#pragma once

#include "orbitersdk.h"
#include <queue>
#include "StackExportStructs.h"
class StackImport
{
public:
	StackImport(ImportData *data);
	~StackImport();
	void prepareImportData();

private:
	void readStack(std::vector<VesselExport> &container, VESSEL *v, VESSEL *caller = NULL, UINT callerportindex = 0, UINT callerindex = 0);

	ImportData *_data;
};

