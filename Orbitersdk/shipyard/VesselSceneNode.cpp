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
		dockingPorts[i].portNode->getMaterial(0).AmbientColor.set(255, 255, 255, 0);
		dockingPorts[i].portNode->getMaterial(0).EmissiveColor.set(150, 150, 150, 150);
		dockingPorts[i].portNode->setVisible(false);

		//rotate portNode so it has the actual orientation of the dockport (direction and up)
		core::matrix4 matrix;
		//it's not a camera, but basically the same thing... except in reverse.
		matrix.buildCameraLookAtMatrixLH(core::vector3df(0, 0, 0), dockingPorts[i].approachDirection, dockingPorts[i].referenceDirection).makeInverse();
		dockingPorts[i].portNode->setRotation(matrix.getRotationDegrees());

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
		if (DEBUG)
		{
			drawDockingPortLines(driver);
		}
	}
}

void VesselSceneNode::drawDockingPortLines(video::IVideoDriver* driver)
{
	for (unsigned int i = 0; i < dockingPorts.size(); i++)
	{
		core::vector3df position = dockingPorts[i].position;
		//draw ddocking direction
		driver->setMaterial(video::SMaterial());
		driver->draw3DLine(position, (5 * dockingPorts[i].approachDirection) + position, video::SColor(255, 255, 0, 0));
		//and draw up vector (up vector is smaller)
		driver->draw3DLine(dockingPorts[i].position, (3 * dockingPorts[i].referenceDirection) + position, video::SColor(255, 0, 255, 0));
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

	ISceneNode* ourNode = ourPort.portNode;
	ISceneNode* theirNode = theirPort.portNode;

	theirNode->updateAbsolutePosition();
	ourNode->updateAbsolutePosition();

	//absolute rotation of the target port
	core::matrix4 theirMatrix = theirNode->getAbsoluteTransformation();

	//Origin up and facing (inversed) of the target port
	core::vector3df theirDir = core::vector3df(0, 0, -1);
	core::vector3df theirRot = core::vector3df(0, 1, 0);

	//get absolute inversed facing and up direction of the target port
	theirMatrix.rotateVect(theirDir);
	theirMatrix.rotateVect(theirRot);
	theirDir.normalize();
	theirRot.normalize();

	//build rotation matrix to rotate from the ORIGIN of the source port to their ports current alignement
	core::matrix4 ourPortToTheirPort;
	ourPortToTheirPort.buildCameraLookAtMatrixLH(core::vector3df(0, 0, 0), theirDir, theirRot).makeInverse();

	//get inverted source port rotation relative to its vessel
	core::matrix4 ourVesselToOurPort = ourNode->getRelativeTransformation();
	ourVesselToOurPort.makeInverse();

	//multiply the rotation from our vessel origin to our port and from our port origin to the target to get the total transformation for the vessel
	core::matrix4 ourVesselToTheirPort = ourPortToTheirPort * ourVesselToOurPort;

	//apply the whole brouhaha
	setRotation(ourVesselToTheirPort.getRotationDegrees());

	//we MUST update positions for getAbsolutePosition to reflect the rotation we just did.
	//also, we MUST update the position of the parent before the child, or the child still won't reflect the changes
	//we also MUST update the child seperately, it doesn't get updated by the parent
	updateAbsolutePosition();
	ourNode->updateAbsolutePosition();

	//position the vessel so the docking ports touch
	core::vector3df pos = ourNode->getAbsolutePosition() - getAbsolutePosition();
	setPosition(theirNode->getAbsolutePosition() - pos);
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