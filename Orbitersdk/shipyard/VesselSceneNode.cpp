#include "VesselSceneNode.h"

VesselSceneNode::VesselSceneNode(string configFilename, scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id)
: scene::ISceneNode(parent, mgr, id), smgr(mgr)
{
	vector<string> tokens;
	ifstream configFile = ifstream(configFilename.c_str());

	bool readingDockingPorts = false;

	while (Helpers::readLine(configFile, tokens))
	{
		//check to see if there are any tokens
		if (tokens.size() == 0)
			continue;

		//or if it is the end
		if (tokens[0].compare("END_DOCKLIST") == 0)
			readingDockingPorts = false;
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
		if (tokens[0].compare("BEGIN_DOCKLIST") == 0)
		{
			readingDockingPorts = true;
		}


		//now see if it is a MeshName
		//put it in lowercase to start
		transform(tokens[0].begin(), tokens[0].end(), tokens[0].begin(), ::tolower);
		//see if it matches
		if (tokens[0].compare("meshname") == 0)
			//load the mesh!
			vesselMesh.setupMesh(string("C:\\Other Stuff\\Orbiter\\shipyard\\Meshes\\" + tokens[2] + ".msh"), mgr->getVideoDriver()); //tokens 2 because the format is
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

void VesselSceneNode::OnRegisterSceneNode()
{
	if (IsVisible)
		smgr->registerNodeForRendering(this);
	ISceneNode::OnRegisterSceneNode();
}

void VesselSceneNode::render()
{
	video::IVideoDriver* driver = SceneManager->getVideoDriver();
	//loop over the mesh groups, drawing them
	for (int i = 0; i < vesselMesh.meshGroups.size(); i++)
	{
		//set the texture of the material
		vesselMesh.materials[vesselMesh.meshGroups[i].materialIndex].setTexture(0,
			vesselMesh.textures[vesselMesh.meshGroups[i].textureIndex]);
		//set the material for the video driver
		driver->setMaterial(vesselMesh.materials[vesselMesh.meshGroups[i].materialIndex]);
		//set transform
		driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
		//and draw it as a triangle list!
		driver->drawVertexPrimitiveList(vesselMesh.meshGroups[i].vertices.data(),
			vesselMesh.meshGroups[i].vertices.size(), vesselMesh.meshGroups[i].triangleList.data(),
			vesselMesh.meshGroups[i].triangleList.size(), video::EVT_STANDARD, scene::EPT_TRIANGLES,
			video::EIT_32BIT);
	}
}

const core::aabbox3d<f32>& VesselSceneNode::getBoundingBox() const
{
	return vesselMesh.boundingBox;
}

u32 VesselSceneNode::getMaterialCount()
{
	return vesselMesh.materials.size();
}

video::SMaterial& VesselSceneNode::getMaterial(u32 i)
{
	return vesselMesh.materials[i];
}