#pragma once
#include "Common.h"
#include "DataManager.h"

//class used to create images of meshes
class SE_PhotoStudio
{
public:
	SE_PhotoStudio(IrrlichtDevice *device);
	~SE_PhotoStudio();

	ITexture *makePicture(VesselData *vesseldata, std::string imagename, bool scene_prepared = false);

private:
	ISceneManager *smgr;
	IVideoDriver *driver;
	gui::IGUIEnvironment *gui;
	ICameraSceneNode *studioCam;
	core::vector3df camRotation;
	float camFOV;
	video::ITexture *canvas;
	int imgSize;

	void setupStudioCam(VesselSceneNode *model);
	ITexture *dumpCanvasToDisk(std::string filename);
};

