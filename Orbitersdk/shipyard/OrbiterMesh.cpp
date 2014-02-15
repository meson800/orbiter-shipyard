#include "OrbiterMesh.h"

OrbiterMesh::OrbiterMesh(string meshFilename)
{
	ifstream meshFile = ifstream(meshFilename.c_str());
	int groupCounter = 0;
	int materialCounter = 0;
	int meshGroupCounter = 0;
	int vertexCounter = 0;
	int triangleCounter = 0;
	int stage = 0;
	string currentLine = 0;

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
			if (strcmp(tokens[0].c_str(), "GROUPS"))
			{
				int numGroups = Helpers::stringToInt(tokens[1]);
				for (int i = 0; i < numGroups; i++)
					meshGroups.push_back(OrbiterMeshGroup());
				break;
			}
			//see if this is our material/texture index
			if (strcmp(tokens[0].c_str(), "MATERIAL"))
			{
				//link the material index
				meshGroups[groupCounter].materialIndex = Helpers::stringToInt(tokens[1]);
				break;
			}
			if (strcmp(tokens[0].c_str(), "TEXTURE"))
			{
				//link the texture index
				meshGroups[groupCounter].textureIndex = Helpers::stringToInt(tokens[1]);
				break;
			}

			//see if this is the geometry one
			if (strcmp(tokens[0].c_str(), "GEOM"))
			{
				//increment the counters
				vertexCounter = Helpers::stringToInt(tokens[1]);
				triangleCounter = Helpers::stringToInt(tokens[2]);
				break;
			}

			//now see if this is a vertex
			if (vertexCounter > 0 && triangleCounter > 0)
			{
				//it's a vertex!
				meshGroups[groupCounter].vertices.push_back(video::S3DVertex(
					Helpers::stringToDouble(tokens[1]), Helpers::stringToDouble(tokens[2]),
					Helpers::stringToDouble(tokens[3]), Helpers::stringToDouble(tokens[4]),
					Helpers::stringToDouble(tokens[5]), Helpers::stringToDouble(tokens[6]),
					video::SColor(),Helpers::stringToDouble(tokens[7]), Helpers::stringToDouble(tokens[8])));
				vertexCounter--;
				break;
			}
			if (vertexCounter == 0 && triangleCounter > 0)
			{
				//it's a triangle!
				meshGroups[groupCounter].triangleList.push_back(make_tuple(Helpers::stringToInt(tokens[1]),
					Helpers::stringToInt(tokens[2]), Helpers::stringToInt(tokens[3])));
				triangleCounter--;
				if (triangleCounter == 0)
					//we finished this meshgroup!
					groupCounter++;
				break;
			}
			//check if we are done with meshgroups

		}
	}
}