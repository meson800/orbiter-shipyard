//Copyright (c) 2015 Christopher Johnstone(meson800) and Benedict Haefeli(jedidia)
//The MIT License - See ../../LICENSE for more info
#pragma once
#include "Common.h"
#include "DataManager.h"

//class used to create images of meshes
class SE_PhotoStudio
{
public:
	SE_PhotoStudio(IrrlichtDevice *device);
	~SE_PhotoStudio();

	ITexture *makePicture(VesselData *vesseldata, std::string imagename);

private:
	ISceneManager *smgr;
	IVideoDriver *driver;
	gui::IGUIEnvironment *gui;
	ICameraSceneNode *studioCam;
	core::vector3df camRotation;
	scene::ILightSceneNode *studioLight;
	float camFOV;
	video::ITexture *canvas;
	int imgSize;

	void setupStudioCam(VesselSceneNode *model);
	ITexture *dumpCanvasToDisk(std::string filename);
};

