#include "VesselSceneNode.h"

VesselSceneNode::VesselSceneNode(std::string configFilename, scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id)
: scene::ISceneNode(parent, mgr, id), smgr(mgr)
{
	vector<std::string> tokens;
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
			vesselMesh->setupMesh(string(Helpers::workingDirectory + "\\Meshes\\" + tokens[2] + ".msh"), mgr->getVideoDriver()); //tokens 2 because the format is
		//MeshName = blahblah

		//clear tokens
		tokens.clear();
	}
	//setup docking port nodes
	setupDockingPortNodes();
}

VesselSceneNode::VesselSceneNode(VesselData *vesData, scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id)
: scene::ISceneNode(parent, mgr, id), smgr(mgr)
{
	vesselData = vesData;
	vesselMesh = vesselData->vesselMesh;
	dockingPorts = vesselData->dockingPorts;

	setupDockingPortNodes();
}

void VesselSceneNode::setupDockingPortNodes()
{
	for (int i = 0; i < dockingPorts.size(); i++)
	{
		dockingPorts[i].parent = this;
		dockingPorts[i].docked = false;

		dockingPorts[i].portNode = smgr->addSphereSceneNode((f32)1.4, 16, this, DOCKPORT_ID, dockingPorts[i].position);
		/*attempt to rotate dockingport nodes to appropriate direction, so dockport orientation could be derived with getAbsoluteTransformation. Results inconsistent, will take another look later*/
/*		core::matrix4 matrix;
		matrix.buildCameraLookAtMatrixLH(core::vector3df(0, 0, 0), dockingPorts[i].approachDirection, dockingPorts[i].referenceDirection);
		dockingPorts[i].portNode->setRotation(matrix.getRotationDegrees());*/

		dockingPorts[i].portNode->getMaterial(0).AmbientColor.set(255, 255, 255, 0);
		dockingPorts[i].portNode->getMaterial(0).EmissiveColor.set(150, 150, 150, 150);
		dockingPorts[i].portNode->setVisible(false);

		//the helper node is used to avoid collision conflicts when checking for visual overlap between the mousecursor and docking nodes
		//in short, the currently selected stack turns on the helper nodes to avoid stealing the overlap event from other vessels
		dockingPorts[i].helperNode = smgr->addSphereSceneNode((f32)1.4, 16, this, HELPER_ID, dockingPorts[i].position);
		dockingPorts[i].helperNode->getMaterial(0).AmbientColor.set(255, 255, 255, 0);
		dockingPorts[i].helperNode->getMaterial(0).EmissiveColor.set(150, 150, 150, 150);
		dockingPorts[i].helperNode->setVisible(false);
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
	for (int i = 0; i < vesselMesh->meshGroups.size(); i++)
	{
		//set the texture of the material
		//set it to zero if there is no texture
		if (vesselMesh->meshGroups[i].textureIndex == 0)
			vesselMesh->materials[vesselMesh->meshGroups[i].materialIndex].setTexture(0, 0);
		else if (vesselMesh->meshGroups[i].materialIndex < vesselMesh->materials.size() &&
			vesselMesh->meshGroups[i].textureIndex < vesselMesh->textures.size())
			vesselMesh->materials[vesselMesh->meshGroups[i].materialIndex].setTexture(0,
				vesselMesh->textures[vesselMesh->meshGroups[i].textureIndex]);
		//set the material for the video driver
		if (vesselMesh->meshGroups[i].materialIndex < vesselMesh->materials.size())
			driver->setMaterial(vesselMesh->materials[vesselMesh->meshGroups[i].materialIndex]);
		//set transform
		driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);
		//and draw it as a triangle list!
		driver->drawVertexPrimitiveList(vesselMesh->meshGroups[i].vertices.data(),
			vesselMesh->meshGroups[i].vertices.size(), vesselMesh->meshGroups[i].triangleList.data(),
			vesselMesh->meshGroups[i].triangleList.size() / 3, video::EVT_STANDARD, scene::EPT_TRIANGLES,
			video::EIT_32BIT);
	}
}

const core::aabbox3d<f32>& VesselSceneNode::getBoundingBox() const
{
	return vesselMesh->boundingBox;
}

u32 VesselSceneNode::getMaterialCount()
{
	return vesselMesh->materials.size();
}

video::SMaterial& VesselSceneNode::getMaterial(u32 i)
{
	return vesselMesh->materials[i];
}

void VesselSceneNode::changeDockingPortVisibility(bool showEmpty, bool showDocked, bool showHelper)
{
	for (unsigned int i = 0; i < dockingPorts.size(); i++)
	{
		if (showHelper)
		//show the helper node instead of the port node. 
		{
			if (dockingPorts[i].docked == false)
				dockingPorts[i].helperNode->setVisible(showEmpty);
			else
				dockingPorts[i].helperNode->setVisible(showDocked);
		}
		else
		{
			if (dockingPorts[i].docked == false)
				dockingPorts[i].portNode->setVisible(showEmpty);
			else
				dockingPorts[i].portNode->setVisible(showDocked);
		}
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

	//attempt at a more stable snapping routine. results inconsistent, but partially tracked to not correctly aligned portNodes. Will take another stab at it down the line
	/*	ISceneNode* ourNode = ourPort.portNode;
	ISceneNode* theirNode = theirPort.portNode;
	theirNode->updateAbsolutePosition();
	ourNode->updateAbsolutePosition();

	//place the module so the docking ports touch
	core::vector3df pos = ourNode->getAbsolutePosition() - getAbsolutePosition();
	setPosition(theirNode->getAbsolutePosition() - pos);

	//get the rotation needed to make our port face the other port
	core::matrix4 theirMatrix = theirNode->getAbsoluteTransformation();
	core::vector3df theirInversedDir = core::vector3df(0, 0, -1);		//as we need to be facing the port
	core::vector3df theirRot = core::vector3df(0, 1, 0);
	theirMatrix.rotateVect(theirInversedDir);
	theirMatrix.rotateVect(theirRot);

	core::matrix4 ourPortToTheirPort;
	ourPortToTheirPort.buildCameraLookAtMatrixLH(core::vector3df(0,0,0), theirInversedDir, theirRot);
	setRotation(ourPortToTheirPort.getRotationDegrees());*/

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

	core::vector3df rotationInEuler;
	finalRotation.toEuler(rotationInEuler);
	//multiply to get it back in degrees
	rotationInEuler *= core::RADTODEG;

	//first rotate around this direction
	//make sure the w isn't zero or that we have an empty vector
	if (!(finalRotation.W == 0 || (finalRotation.X == 0 && finalRotation.Y == 0 && finalRotation.Z == 0)))
		setRotation(getRotation() + rotationInEuler);

	//NOTE: APPLY REF DIRECTION ROTATION
	core::quaternion refDirectionRotation;
	//find the quaternoin
	refDirectionRotation.rotationFromTo(ourPort.parent->returnRotatedVector(ourPort.referenceDirection),
		theirPort.parent->returnRotatedVector(theirPort.referenceDirection));



	//apply this ref direction rotation
	refDirectionRotation.toEuler(rotationInEuler);
	//multiply to get it back in degrees
	rotationInEuler *= core::RADTODEG;

	//apply rotation
	if (!(refDirectionRotation.W == 0 || (refDirectionRotation.X == 0 && refDirectionRotation.Y == 0 &&refDirectionRotation.Z == 0)))
		setRotation(getRotation() + rotationInEuler);

	//now move the thing to match with it
	//set our position to the difference between the docking ports- theirs minus ours
	core::vector3df startPosition = getAbsolutePosition();
	core::vector3df difference = (theirPort.parent->getAbsolutePosition() + theirPort.parent->returnRotatedVector(theirPort.position))
		- (getAbsolutePosition() + returnRotatedVector(ourPort.position));
	setPosition(startPosition + difference);


}

void VesselSceneNode::dock(OrbiterDockingPort& ourPort, OrbiterDockingPort& theirPort)
{
	//set both docked flags
	ourPort.docked = true;
	theirPort.docked = true;
	//set dockedTo pointers
	ourPort.dockedTo = &theirPort;
	theirPort.dockedTo = &ourPort;
}