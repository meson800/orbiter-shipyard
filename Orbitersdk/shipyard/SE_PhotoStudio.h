#pragma once
using namespace irr::core;

//class used to create images of meshes
class SE_PhotoStudio
{
public:
	SE_PhotoStudio(IrrlichtDevice *device);
	~SE_PhotoStudio();

	void makePicture(VesselData *vesseldata, bool scene_prepared = false);

private:
	ISceneManager *smgr;
	IVideoDriver *driver;
	ICameraSceneNode *studioCam;
	vector3df camRotation;
	float camFOV;
	video::ITexture *canvas;
	int imgSize;

	void setupStudioCam(VesselSceneNode *model);
	void dumpCanvasToDisk(std::string filename);
};

