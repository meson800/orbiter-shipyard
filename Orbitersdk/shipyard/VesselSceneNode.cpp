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
			core::vector3d<f32>(Helpers::stringToDouble(tokens[0]),
				Helpers::stringToDouble(tokens[1]), Helpers::stringToDouble(tokens[2])),
			core::vector3d<f32>(Helpers::stringToDouble(tokens[3]),
			Helpers::stringToDouble(tokens[4]), Helpers::stringToDouble(tokens[5])),
				core::vector3d<f32>(Helpers::stringToDouble(tokens[6]),
			Helpers::stringToDouble(tokens[7]), Helpers::stringToDouble(tokens[8]))));
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
			vesselMesh.setupMesh(string(Helpers::workingDirectory + "\\Meshes\\" + tokens[2] + ".msh"), mgr->getVideoDriver(), smgr); //tokens 2 because the format is
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
	{
		dockingPorts[i].parent = this;
		dockingPorts[i].docked = false;
		dockingPorts[i].portNode = smgr->addSphereSceneNode((f32)1.2, 16, this, ID_Flag_IsDockingPort, dockingPorts[i].position);
		dockingPorts[i].portNode->getMaterial(0).AmbientColor.set(255,255,255,0);
		dockingPorts[i].portNode->getMaterial(0).EmissiveColor.set(150, 150, 150, 150);
		dockingPorts[i].portNode->setVisible(false);
	}

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
		//set it to zero if there is no texture
		if (vesselMesh.meshGroups[i].textureIndex == 0)
			vesselMesh.materials[vesselMesh.meshGroups[i].materialIndex].setTexture(0, 0);
		else if (vesselMesh.meshGroups[i].materialIndex < vesselMesh.materials.size() &&
			vesselMesh.meshGroups[i].textureIndex < vesselMesh.textures.size())
			vesselMesh.materials[vesselMesh.meshGroups[i].materialIndex].setTexture(0,
				vesselMesh.textures[vesselMesh.meshGroups[i].textureIndex]);
		//set the material for the video driver
		if (vesselMesh.meshGroups[i].materialIndex < vesselMesh.materials.size())
			driver->setMaterial(vesselMesh.materials[vesselMesh.meshGroups[i].materialIndex]);
		//set transform
		driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
		//and draw it as a triangle list!
		driver->drawVertexPrimitiveList(vesselMesh.meshGroups[i].vertices.data(),
			vesselMesh.meshGroups[i].vertices.size(), vesselMesh.meshGroups[i].triangleList.data(),
			vesselMesh.meshGroups[i].triangleList.size() / 3, video::EVT_STANDARD, scene::EPT_TRIANGLES,
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

void VesselSceneNode::changeDockingPortVisibility(bool show, bool emptyOnly)
{
	for (unsigned int i = 0; i < dockingPorts.size(); i++)
	{
		//if emptyOnly is false, always show/hide node
		//if emptyOnly is true, only show if this one is empty
		if (emptyOnly == false || dockingPorts[i].docked == false)
			dockingPorts[i].portNode->setVisible(show);
	}
}

core::vector3df VesselSceneNode::returnRotatedVector(const core::vector3df& vec)
{
	//update our absolute position
	updateAbsolutePosition();
	//first, rotate the vector of the approach port by our current rotation
	//do this by making a quaternoin
	core::quaternion thisRotation = core::quaternion(getRotation() * core::DEGTORAD);
	//now return a rotated vector
	return thisRotation * vec;
}

void VesselSceneNode::snap(OrbiterDockingPort& ourPort, OrbiterDockingPort& theirPort)
{
	//ok, this gets complicated
	//update our absolute position
	updateAbsolutePosition();
	theirPort.parent->updateAbsolutePosition();
	//this is the eventual final quaternion rotation
	core::quaternion finalRotation;
	
	//get the rotated port direction, times -1 to reverse it so it will match with the other port
	core::vector3df ourPortDirection = -1 * returnRotatedVector(ourPort.approachDirection);
	//get the other rotated port vector
	core::vector3df otherPortDirection = ((VesselSceneNode*)theirPort.parent)->returnRotatedVector(theirPort.approachDirection);
	//get our first quaternion
	finalRotation.rotationFromTo(ourPortDirection, otherPortDirection);

	//NOTE: APPLY REF DIRECTION ROTATION

	//apply this final rotation
	core::vector3df rotationInEuler;
	finalRotation.toEuler(rotationInEuler);
	//multiply to get it back in degrees
	rotationInEuler *= core::RADTODEG;

	//make sure the w isn't zero
	if (finalRotation.W != 0)
		setRotation(getRotation() + rotationInEuler);

	//now move the thing to match with it
	//set our position to the difference between the docking ports- theirs minus ours
	core::vector3df startPosition = getAbsolutePosition();
	core::vector3df difference = (theirPort.parent->getAbsolutePosition() + theirPort.parent->returnRotatedVector(theirPort.position))
		- (getAbsolutePosition() + returnRotatedVector(ourPort.position));
	setPosition(startPosition + difference);


}