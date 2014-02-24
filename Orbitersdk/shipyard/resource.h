#pragma once

#define VESSEL_ID 72

enum NodeId
{
	ID_IsNotPickable = 0,
	ID_IsPickable = 1 << 0,
	ID_Flag_IsDockingPort = 1 << 1
};