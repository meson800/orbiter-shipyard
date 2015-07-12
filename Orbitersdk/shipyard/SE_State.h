#pragma once

#include <map>
#include <irrlicht.h>

#include "Helpers.h"
#include "VesselSceneNode.h"

class SE_GlobalState
{
public:
    SE_GlobalState() {}
    SE_GlobalState(const std::map<UINT, VesselSceneNode*>& vessels);
    void apply(scene::ISceneManager* mgr);
    std::map<UINT, VesselSceneNodeState> state;
};

struct VesselDiffState
{
    enum DiffType {ADD,UPDATE,DELETE} diffType;
    VesselSceneNodeState state;
};

class SE_DiffState
{
public:
    SE_DiffState(const SE_GlobalState& oldState, const SE_GlobalState& newState);
    void apply(scene::ISceneManager* mgr);

private:
    std::map <UINT, VesselDiffState> state;
};