#include "VesselSceneNode.h"

VesselSceneNode::VesselSceneNode(string configFilename, scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id)
: scene::ISceneNode(parent, mgr, id), smgr(mgr)
{
	vector<string> tokens;
	ifstream configFile = ifstream(configFilename.c_str());

	bool readingDockingPorts = false;

	while (Helpers::readLine(configFile, tokens))
	{
		//if we are reading docking ports, create a new docking port!
		if (readingDockingPorts)
			dockingPorts.push_back(OrbiterDockingPort(
			core::vector3d<f32>(Helpers::stringToDouble(tokens[1]),
				Helpers::stringToDouble(tokens[2]), Helpers::stringToDouble(tokens[3])),
			core::vector3d<f32>(Helpers::stringToDouble(tokens[4]),
			Helpers::stringToDouble(tokens[5]), Helpers::stringToDouble(tokens[6])),
				core::vector3d<f32>(Helpers::stringToDouble(tokens[7]),
			Helpers::stringToDouble(tokens[8]), Helpers::stringToDouble(tokens[9]))));
		//now see if this is the beginning of a docking port list
		if (tokens[0].compare("BEGIN_DOCKLIST"))
		{
			readingDockingPorts = true;
		}
		//or if it is the end
		if (tokens[0].compare("END_DOCKLIST"))
			readingDockingPorts = false;

		//now see if it is a MeshName
		//put it in lowercase to start
		transform(tokens[0].begin(), tokens[0].end(), tokens[0].begin(), ::tolower);
		//see if it matches
		if (tokens[0].compare("meshname"))
			//load the mesh!
			vesselMesh.setupMesh(string(".\\Meshes\\" + tokens[2]), mgr->getVideoDriver()); //tokens 2 because the format is
		//MeshName = blahblah

		//clear tokens
		tokens.clear();
	}
	//setup docking port nodes
	setupDockingPortNodes();
}

void VesselSceneNode::setupDockingPortNodes()
{
	for (int i = 0; i < dockingPorts.size(); i++)
		dockingPortNodes.push_back(smgr->addSphereSceneNode(10, 16, this, 10, dockingPorts[i].position));
}

