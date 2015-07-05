#pragma once
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

