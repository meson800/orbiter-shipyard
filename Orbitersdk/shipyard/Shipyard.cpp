#include "Shipyard.h"

void Shipyard::loop()
{
	video::IVideoDriver* driver = device->getVideoDriver();
	scene::ISceneManager* smgr = device->getSceneManager();
	
	//add the camera
	smgr->addCameraSceneNodeMaya();

	//register our VesselSceneNode - just staticly at the moment, but will do it later
	VesselSceneNode* vesselNode = new VesselSceneNode("C:\\Other Stuff\\Orbiter\\shipyard\\Config\\Vessels\\ISS.cfg", 
		smgr->getRootSceneNode(), smgr, 1);
	vesselNode->drop();

	//start the loop
	while (device->run())
	{
		driver->beginScene(true, true, video::SColor(0, 100, 100, 100));

		smgr->drawAll();

		driver->endScene();
	}
}