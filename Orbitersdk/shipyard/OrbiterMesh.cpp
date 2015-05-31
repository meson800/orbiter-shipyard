#include "OrbiterMesh.h"

OrbiterMesh::OrbiterMesh()
{}

OrbiterMesh::OrbiterMesh(string meshFilename, video::IVideoDriver* driver, scene::ISceneManager* smgr)
{
	setupMesh(meshFilename, driver);
}

bool OrbiterMesh::setupMesh(string meshFilename, video::IVideoDriver* driver)
{
	ifstream meshFile = ifstream(meshFilename.c_str());
	if (!meshFile) return false;

	int groupCounter = 0;
	bool noNormal = false;
	int materialCounter = 1;			//a default material will be added in any case right below
	int textureCounter = 0;				//a default texture will be added in any case right below
	int meshGroupCounter = 0;
	int vertexCounter = 0;
	int triangleCounter = 0;
	int stage = 0;
	string currentLine;

	//push the default material
	materials.push_back(video::SMaterial());
	//push the default texture
	Helpers::videoDriverMutex.lock();
	textures.push_back(driver->addTexture(core::dimension2d<u32>(1, 1),"empty_texture"));
	Helpers::videoDriverMutex.unlock();

	vector<string> tokens;
	//start reading the meshgroup
	while (Helpers::readLine(meshFile, tokens))
	{
		//split this into stages
		switch (stage)
		{
		case 0:
			//stage zero is defining the meshgroups
			//see if this is the main group definition
			if (strcmp(tokens[0].c_str(), "GROUPS") == 0)
			{
				int numGroups = Helpers::stringToInt(tokens[1]);
				for (int i = 0; i < numGroups; i++)
				{
					meshGroups.push_back(OrbiterMeshGroup());
					//set default
					meshGroups[i].materialIndex = 0;
					meshGroups[i].textureIndex = 0;
				}
				break;
			}
			//see if this is our material/texture index
			if (strcmp(tokens[0].c_str(), "MATERIAL") == 0)
			{
				//link the material index
				meshGroups[groupCounter].materialIndex = Helpers::stringToInt(tokens[1]);
				break;
			}
			if (strcmp(tokens[0].c_str(), "TEXTURE") == 0)
			{
				//link the texture index
				meshGroups[groupCounter].textureIndex = Helpers::stringToInt(tokens[1]);
				break;
			}
			//see if there aren't going to be normals
			if (strcmp(tokens[0].c_str(), "NONORMAL") == 0)
				noNormal = true;

			//see if this is the geometry one
			if (strcmp(tokens[0].c_str(), "GEOM") == 0)
			{
				//increment the counters
				vertexCounter = Helpers::stringToInt(tokens[1]);
				triangleCounter = Helpers::stringToInt(tokens[2]);
				break;
			}

			//now see if this is a vertex
			if (vertexCounter > 0 && triangleCounter > 0)
			{
				//if there are no normals
				if (tokens.size() <= 3)
					noNormal = true;
				//see if there are no normals
				if (noNormal)
				{
					//see if there are at least 4 values-this tells us there are texture coords
					if (tokens.size() > 4)
					{
						//insert empty things into 3,4,5
						for (int i = 3; i <= 5; i++)
							tokens.insert(tokens.begin()+3, "0");
					}
					//whoops, just insert these at the end
					else
					{
						for (int i = 3; i <= 5; i++)
							tokens.push_back("0");
					}
				}

				//now see if there are texture coords
				if (tokens.size() < 7)
				{
					//insert empty texture coords
					for (int i = 0; i < 2; i++)
						tokens.push_back("0");
				}
				//it's a vertex!
				meshGroups[groupCounter].vertices.push_back(video::S3DVertex(
					Helpers::stringToDouble(tokens[0]), Helpers::stringToDouble(tokens[1]),
					Helpers::stringToDouble(tokens[2]), Helpers::stringToDouble(tokens[3]),
					Helpers::stringToDouble(tokens[4]), Helpers::stringToDouble(tokens[5]),
					video::SColor(255,255,255,255), Helpers::stringToDouble(tokens[6]), Helpers::stringToDouble(tokens[7])));
				
				vertexCounter--;
				break;
			}
			if (vertexCounter == 0 && triangleCounter > 0)
			{
				//it's a triangle!
				meshGroups[groupCounter].triangleList.push_back(Helpers::stringToInt(tokens[0]));
				meshGroups[groupCounter].triangleList.push_back(Helpers::stringToInt(tokens[1]));
				meshGroups[groupCounter].triangleList.push_back(Helpers::stringToInt(tokens[2]));
				triangleCounter--;
				if (triangleCounter == 0)
				{
					//see if we need to calculate normals
					if (noNormal)
						setupNormals(groupCounter);
					//we finished this meshgroup!
					groupCounter++;
					//reset nonormal flag
					noNormal = false;

					//check if we are done with meshgroups
					if (groupCounter == meshGroups.size() && meshGroups.size() != 0)
					{
						//we're done reading meshgroups!!!
						stage++;
						break;
					}
				}

				break;
			}

			//something went wrong, just break
			break;

		case 1:
			//now we are reading materials
			//see if this is the main Material line with the number of materials
			if (strcmp(tokens[0].c_str(), "MATERIALS") == 0)
			{
				int numMaterials = Helpers::stringToInt(tokens[1]);
				for (int i = 0; i < numMaterials; i++)
				{
					materials.push_back(video::SMaterial());
				}
				break;
			}
			//otherwise see if this is the main material declaration
			if (strcmp(tokens[0].c_str(), "MATERIAL") == 0)
			{
				//clear whatever is in this line
				tokens.clear();

				//read the next four lines
				for (int i = 0; i < 4; i++)
					Helpers::readLine(meshFile, tokens);
				//now the next 17 values should be in tokens[2]-tokens[18]

				//insert standard shininess if it doesn't exist
				if (tokens.size() == 16)
					tokens.insert(tokens.begin() + 12, "20");

				//orbiter format
				//0		1	2	3		Diffuse colour(RGBA)
				//4		5	6	7		Ambient colour(RGBA)
				//8		9	10	11	12 Specular colour(RGBA) and specular power(float)
				//13	14	15	16		Emissive colour(RGBA)
				materials[materialCounter].DiffuseColor.setRed(Helpers::stringToDouble(tokens[0]) * 255);
				materials[materialCounter].DiffuseColor.setGreen(Helpers::stringToDouble(tokens[1]) * 255);
				materials[materialCounter].DiffuseColor.setBlue(Helpers::stringToDouble(tokens[2]) * 255);
				materials[materialCounter].DiffuseColor.setAlpha(Helpers::stringToDouble(tokens[3]) * 255);

				materials[materialCounter].AmbientColor.setRed(Helpers::stringToDouble(tokens[4]) * 255);
				materials[materialCounter].AmbientColor.setGreen(Helpers::stringToDouble(tokens[5]) * 255);
				materials[materialCounter].AmbientColor.setBlue(Helpers::stringToDouble(tokens[6]) * 255);
				materials[materialCounter].AmbientColor.setAlpha(Helpers::stringToDouble(tokens[7]) * 255);

				materials[materialCounter].SpecularColor.setRed(Helpers::stringToDouble(tokens[8]) * 255);
				materials[materialCounter].SpecularColor.setGreen(Helpers::stringToDouble(tokens[9]) * 255);
				materials[materialCounter].SpecularColor.setBlue(Helpers::stringToDouble(tokens[10]) * 255);
				materials[materialCounter].SpecularColor.setAlpha(Helpers::stringToDouble(tokens[11]) * 255);
				//set specular power-"shineness"
				materials[materialCounter].Shininess = Helpers::stringToDouble(tokens[12]);

				materials[materialCounter].EmissiveColor.setRed(Helpers::stringToDouble(tokens[13]) * 255);
				materials[materialCounter].EmissiveColor.setGreen(Helpers::stringToDouble(tokens[14]) * 255);
				materials[materialCounter].EmissiveColor.setBlue(Helpers::stringToDouble(tokens[15]) * 255);
				materials[materialCounter].EmissiveColor.setAlpha(Helpers::stringToDouble(tokens[16]) * 255);


				//we're done!
				materialCounter++;
				if (materialCounter == materials.size() && materials.size() != 0)
					stage++;
				break;
			}
			break;
		case 2:
			//now we are loading textures
			if (strcmp(tokens[0].c_str(), "TEXTURES") == 0)
			{
				textureCounter = Helpers::stringToInt(tokens[1]);
				break;
			}

			//see if this is a texture path
			if (textureCounter > 0)
			{
				textures.push_back(Helpers::readDDS(string(Helpers::workingDirectory + "\\Textures\\" + tokens[0]).c_str(),
					tokens[0].c_str(),driver));
				textureCounter--;
				break;
			}
			break;

		}

		//clean up the token vector
		tokens.clear();
	}

	//now, set up the bounding box
	boundingBox.reset(meshGroups[0].vertices[0].Pos);
	//loop over every vertex
	for (int i = 0; i < meshGroups.size(); i++)
	{
		for (int j = 0; j < meshGroups[i].vertices.size(); j++)
			boundingBox.addInternalPoint(meshGroups[i].vertices[j].Pos);
	}
	return true;
}

void OrbiterMesh::setupNormals(int meshGroup)
{
	//reset all normals in this mesh group just in case
	for (int i = 0; i < meshGroups[meshGroup].vertices.size(); i++)
		meshGroups[meshGroup].vertices[i].Normal = core::vector3df(0, 0, 0);

	//calculate normal
	for (int i = 0; i < meshGroups[meshGroup].triangleList.size() / 3; i++)
	{
		core::vector3df firstvec = meshGroups[meshGroup].vertices[meshGroups[meshGroup].triangleList[i * 3 + 1]].Pos -
			meshGroups[meshGroup].vertices[meshGroups[meshGroup].triangleList[i * 3]].Pos;
		core::vector3df secondvec = meshGroups[meshGroup].vertices[meshGroups[meshGroup].triangleList[i * 3]].Pos - 
			meshGroups[meshGroup].vertices[meshGroups[meshGroup].triangleList[i * 3 + 2]].Pos;
		core::vector3df normal = firstvec.crossProduct(secondvec);
		normal = normal.normalize();
		meshGroups[meshGroup].vertices[meshGroups[meshGroup].triangleList[i * 3]].Normal += normal;
		meshGroups[meshGroup].vertices[meshGroups[meshGroup].triangleList[i * 3 + 1]].Normal += normal;
		meshGroups[meshGroup].vertices[meshGroups[meshGroup].triangleList[i * 3 + 2]].Normal += normal;
	}

	//normalize the normals
	for (int i = 0; i < meshGroups[meshGroup].vertices.size(); i++)
		meshGroups[meshGroup].vertices[i].Normal = meshGroups[meshGroup].vertices[i].Normal.normalize();
}