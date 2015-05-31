#pragma once

//nodetype identifiers in the form of bitmasks
enum
{
	VESSEL_ID = 1 << 0,
	DOCKPORT_ID = 1 << 1,
	HELPER_ID = 1 << 2
};

/*enum NodeId
{
	ID_IsNotPickable = 0,
	ID_IsPickable = 1 << 0,
	ID_Flag_IsDockingPort = 1 << 1
};*/