#include "SE_State.h"

SE_GlobalState::SE_GlobalState(const std::map<UINT, VesselSceneNode*>& vessels)
{
    for (auto it = vessels.begin(); it != vessels.end(); ++it)
        state[it->first] = it->second->saveState();
}

void SE_GlobalState::apply(scene::ISceneManager* mgr)
{
    core::list<ISceneNode*> allNodes = mgr->getRootSceneNode()->getChildren();
    //clear all old vessels
    std::vector<ISceneNode*> childrenToDelete;
    for (auto it = allNodes.begin(); it != allNodes.end(); ++it)
    {
        if ((*it)->getID() == VESSEL_ID)
            childrenToDelete.push_back(*it);
    }
    for (UINT i = 0; i < childrenToDelete.size(); ++i)
    {
        mgr->getRootSceneNode()->removeChild(childrenToDelete[i]);
    }

    //create new vessels
    for (auto it = state.begin(); it != state.end(); ++it)
        VesselSceneNode* newVessel = new VesselSceneNode(it->second, mgr->getRootSceneNode(), mgr, VESSEL_ID);
}

SE_DiffState::SE_DiffState(const SE_GlobalState& oldState, const SE_GlobalState& newState)
{
    //check through every vessel in old state
    for (auto it = oldState.state.begin(); it != oldState.state.end(); ++it)
    {
        VesselDiffState currentVesselDiffState;
        //if this vessel UID exists in the new state, this is an update
        if (newState.state.count(it->first))
        {
            currentVesselDiffState.diffType = VesselDiffState::DiffType::UPDATE;
            currentVesselDiffState.state = newState.state.at(it->first);
        }
        else
        {
            //otherwise this is a delete operation
            currentVesselDiffState.diffType = VesselDiffState::DiffType::DELETE;
        }
        state[it->first] = currentVesselDiffState;
    }

    //now check for new vessels in newState, those which aren't in oldState
    for (auto it = newState.state.begin(); it != newState.state.end(); ++it)
    {
        if (!oldState.state.count(it->first))
        {
            //old state does not contain this vessel, so add operation
            VesselDiffState currentVesselDiffState;
            currentVesselDiffState.diffType = VesselDiffState::DiffType::ADD;
            currentVesselDiffState.state = it->second;
            state[it->first] = currentVesselDiffState;
        }
    }
}

void SE_DiffState::apply(scene::ISceneManager* mgr)
{
    Log::writeToLog(Log::INFO, "Applying diff state");
    //go through internal state, switching on diff type

    //do ADD and DELETE operations first, so updates with docking work
    for (auto it = state.begin(); it != state.end(); ++it)
    {
        switch (it->second.diffType)
        {
        case VesselDiffState::DiffType::ADD:
        {
            //simply create new vessel
            VesselSceneNode* newVessel = new VesselSceneNode(it->second.state, mgr->getRootSceneNode(), mgr, VESSEL_ID);
            break;
        }
        case VesselDiffState::DiffType::DELETE:
        {
            VesselSceneNode* vessel = Helpers::getVesselByUID(it->first);
            vessel->removeAll(); //removeAll only drops children, not the node itself
            vessel->remove(); //remove from scene graph
            vessel->drop(); //We need the extra drop because we called VesselSceneNode, which returns a pointer
            break;
        }
        default:
            break;
        }
    }
    for (auto it = state.begin(); it != state.end(); ++it)
    {
        switch (it->second.diffType)
        {
        case VesselDiffState::DiffType::UPDATE:
            Log::writeToLog(Log::L_DEBUG, "DiffState: Updating vessel, uid: ", it->first);
            //find current existing vessel and update it
            Helpers::getVesselByUID(it->first)->loadState(it->second.state);
            break;
        default:
            break;
        }
    }

}