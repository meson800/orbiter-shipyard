#include "VesselSceneNode.h"

VesselSceneNodeState::VesselSceneNodeState(VesselData* data, ifstream& file)
{
    vesData = data;
    vector<string> tokens;
    bool done = false;
    while (!done)
    {
        done = !Helpers::readLine(file, tokens);
        if (done)
            //unexpected eof, write to log and abort
        {
            Log::writeToLog(Log::ERR, "Unexpected end of file while loading session");
            throw VesselSceneNodeParseError("Unexpected end of file wile loading from session");
        }
        if (tokens.size() == 0)
            continue;
        if (tokens[0].compare("POS") == 0)
        {
            if (tokens.size() < 4)
                //line doesn't contain enough values
            {
                Log::writeToLog(Log::ERR, "Invalid POS in session");
                throw VesselSceneNodeParseError("Invalid POS in session");
            }

            pos = core::vector3df(Helpers::stringToDouble(tokens[1]),
                Helpers::stringToDouble(tokens[2]),
                Helpers::stringToDouble(tokens[3]));
        }
        else if (tokens[0].compare("ROT") == 0)
        {
            if (tokens.size() < 4)
                //line doesn't contain enough values
            {
                Log::writeToLog(Log::ERR, "Invalid ROT in session");
                throw VesselSceneNodeParseError("Invalid ROT in session");
            }

            rot = core::vector3df(Helpers::stringToDouble(tokens[1]),
                Helpers::stringToDouble(tokens[2]),
                Helpers::stringToDouble(tokens[3]));
        }
        else if (tokens[0].compare("UID") == 0)
        {
            if (tokens.size() < 2)
                //line doesn't contain enough values
            {
                Log::writeToLog(Log::ERR, "Invalid UID in session");
                throw VesselSceneNodeParseError("Invalid UID in session");
            }
            uid = Helpers::stringToInt(tokens[1]);
        }
        else if (tokens[0].compare("ORBITERNAME") == 0)
        {
            if (tokens.size() < 2)
                //line doesn't contain enough values
            {
                Log::writeToLog(Log::WARN, "Invalid ORBITERNAME parameter");
                //loading is still successful. this is a minor issue
            }
            else
            {
                orbiterName = tokens[1];
            }
        }
        else if (tokens[0].compare("VESSEL_END") == 0)
        {
            done = true;
        }
        tokens.clear();
    }
}

void VesselSceneNodeState::saveToFile(ofstream& file)
{
    //writes pos and rot of the vesselscenenode to a session file
    file << "VESSEL_BEGIN\n" << "FILE =  " << vesData->className << "\n";
    file << "POS = " << pos.X << " " << pos.Y << " " << pos.Z << "\n";
    file << "ROT = " << rot.X << " " << rot.Y << " " << rot.Z << "\n";
    file << "UID = " << uid << "\n";
    if (orbiterName != "")
    {
        file << "ORBITERNAME = " << orbiterName << "\n";
    }
    file << "VESSEL_END\n";
}

UINT VesselSceneNode::next_uid = 0;

VesselSceneNode::VesselSceneNode(VesselData *vesData, scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id, UINT _uid)
    : scene::ISceneNode(parent, mgr, id), smgr(mgr), uid(_uid)
{
    Log::writeToLog(Log::INFO, "Creating VesselSceneNode with UID: ", _uid);
	vesselData = vesData;
	vesselMesh = vesselData->vesselMesh;
	dockingPorts = vesselData->dockingPorts;

	setupDockingPortNodes();
    //set own UID
    //currently unsafe, as it doesn't check if the UID is actually unique
    uid = _uid;
    //register self with map
    Helpers::registerVessel(uid, this);
}

VesselSceneNode::VesselSceneNode(const VesselSceneNodeState& state, scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id)
    : VesselSceneNode(state.vesData,parent,mgr,id,state.uid)
{
    loadState(state);
}


VesselSceneNode::~VesselSceneNode()
{
    Log::writeToLog(Log::INFO, "Deleting VesselSceneNode with UID: ", uid);
    //unregister self from map
    Helpers::unregisterVessel(uid);
}

UINT VesselSceneNode::getUID()
{
    return uid;
}

void VesselSceneNode::setupDockingPortNodes()
{
	for (int i = 0; i < dockingPorts.size(); i++)
	{
		dockingPorts[i].parent = this;
		dockingPorts[i].docked = false;
        dockingPorts[i].portID = i;

		dockingPorts[i].portNode = smgr->addSphereSceneNode((f32)1.4, 16, this, DOCKPORT_ID, dockingPorts[i].position);
		setupDockingPortNode(dockingPorts[i].portNode);

		//rotate portNode so it has the actual orientation of the dockport (direction and up)
		core::matrix4 matrix;
		//it's not a camera, but basically the same thing... except in reverse.
		matrix.buildCameraLookAtMatrixLH(core::vector3df(0, 0, 0), dockingPorts[i].approachDirection, dockingPorts[i].referenceDirection).makeInverse();
		dockingPorts[i].portNode->setRotation(matrix.getRotationDegrees());

		//the helper node is used to avoid collision conflicts when checking for visual overlap between the mousecursor and docking nodes
		//in short, the currently selected stack turns on the helper nodes to avoid stealing the overlap event from other vessels
		dockingPorts[i].helperNode = smgr->addSphereSceneNode((f32)1.4, 16, this, HELPER_ID, dockingPorts[i].position);
		setupDockingPortNode(dockingPorts[i].helperNode);
	}
}

void VesselSceneNode::setupDockingPortNode(IMeshSceneNode *node)
{
	//deactivate lighting so we don't get reflections on the nodes and make them transparent
	node->setMaterialFlag(EMF_LIGHTING, false);
	node->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);

	//now that we don't have lighting, we have to set vertex color directly to have any effect
	S3DVertex *vertices = (S3DVertex*)(node->getMesh()->getMeshBuffer(0)->getVertices());
	UINT vertexcount = node->getMesh()->getMeshBuffer(0)->getVertexCount();
	SColor vertexcolor = SColor(255, 13, 161, 247);
	for (UINT i = 0; i < vertexcount; ++i)
	{
		vertices[i].Color = vertexcolor;
	}

	node->setVisible(false);
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
		{
			SMaterial material = vesselMesh->materials[vesselMesh->meshGroups[i].materialIndex];
			if (transparent)
			{
				material.Lighting = false;
				material.MaterialType = EMT_TRANSPARENT_ADD_COLOR;
			}
			driver->setMaterial(material);
		}
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
	//update the new position, in case there's a vessel being snapped to this right next
	updateAbsolutePosition();				
}

void VesselSceneNode::dock(OrbiterDockingPort& ourPort, OrbiterDockingPort& theirPort)
{
    Log::writeToLog(Log::INFO, "Docking ourPort (VUID: ", ourPort.parent->getUID(), " PID: ", ourPort.portID,
        ") to theirPort (VUID: ", theirPort.parent->getUID(), " PID: ", theirPort.portID, ")");

	//set both docked flags
	ourPort.docked = true;
	theirPort.docked = true;
	//set dockedTo pointers
	ourPort.dockedTo.vesselUID = theirPort.parent->getUID();
    ourPort.dockedTo.portID = theirPort.portID;

    theirPort.dockedTo.vesselUID = ourPort.parent->getUID();
    theirPort.dockedTo.portID = ourPort.portID;
}

void VesselSceneNode::dock(UINT ourPortNum, UINT otherVesselUID, UINT otherPortID)
{
    dock(dockingPorts[ourPortNum], Helpers::getVesselByUID(otherVesselUID)->dockingPorts[otherPortID]);
}

VesselSceneNodeState VesselSceneNode::saveState()
{
    Log::writeToLog(Log::L_DEBUG, "Saving VesselSceneNode state, UID: ", uid);
    VesselSceneNodeState output;
    output.vesData = returnVesselData();
    output.uid = uid;
    output.pos = getAbsolutePosition();
    output.rot = getRotation();
    output.orbiterName = orbitername;

    for (UINT i = 0; i < dockingPorts.size(); ++i)
    {
        output.dockingStatus.push_back(dockingPorts[i].returnStatus());
    }
    return output;
}

void VesselSceneNode::loadState(const VesselSceneNodeState& state)
{
    Log::writeToLog(Log::L_DEBUG, "Loading VesselSceneNode state, UID: ", uid);
    if (state.uid != uid)
        throw UID_Mismatch();
    setPosition(state.pos);
    setRotation(state.rot);
    orbitername = state.orbiterName;

    for (UINT i = 0; i < state.dockingStatus.size(); ++i)
    {
        dockingPorts[i].docked = state.dockingStatus[i].docked;
        if (state.dockingStatus[i].docked)
            dock(i, state.dockingStatus[i].dockedTo.vesselUID, state.dockingStatus[i].dockedTo.portID);
    }

}

OrbiterDockingPort* VesselSceneNode::dockingPortSceneNodeToOrbiter(scene::ISceneNode* sceneNode)
{
	for (UINT i = 0; i < dockingPorts.size(); ++i)
	{
		if (sceneNode == dockingPorts[i].portNode)
		{
			return &(dockingPorts[i]);
		}
	}
	return 0;
}

OrbiterDockingPort* VesselSceneNode::dockingPortHelperNodeToOrbiter(scene::ISceneNode* sceneNode)
{
	for (UINT i = 0; i < dockingPorts.size(); ++i)
	{
		if (sceneNode == dockingPorts[i].helperNode)
		{
			return &(dockingPorts[i]);
		}
	}
	return 0;
}

VesselData* VesselSceneNode::returnVesselData()
{
	return vesselData;
}


std::string VesselSceneNode::getClassName()
{
	return vesselData->className;
}

std::string VesselSceneNode::getOrbiterName()
{
	return orbitername;
}

void VesselSceneNode::setOrbiterName(string name)
{
	orbitername = name;
}


void VesselSceneNode::setTransparency(bool transparency)
{
	transparent = transparency;
}

