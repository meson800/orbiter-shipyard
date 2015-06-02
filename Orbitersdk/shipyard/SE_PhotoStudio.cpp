#include "Common.h"
#include "DataManager.h"
#include "VesselSceneNode.h"
#include "SE_PhotoStudio.h"


SE_PhotoStudio::SE_PhotoStudio(IrrlichtDevice *device)
{
	//create our own scenemanager and keep a pointer to the driver
	smgr = device->getSceneManager()->createNewSceneManager();
	driver = device->getVideoDriver();

	//set default camera rotation and field of view(handpicked for decent results, but can be changed)
	camRotation = vector3df(-27, -26, 0);
	camFOV = 30;
	imgSize = 128;

	//lighting level. might need some more fiddling
	smgr->setAmbientLight(video::SColor(0, 150, 150, 150));
	//create the render target
	canvas = driver->addRenderTargetTexture(core::dimension2d<u32>(imgSize, imgSize), "RTT1");
}


SE_PhotoStudio::~SE_PhotoStudio()
{
	//remove the camera from the scene
	studioCam->drop();
}


//makes a picture of a single VesselSceneNode
// vesseldata: the VesselData of the vessel to make a picture from
// scene_prepared: send true if the render target has already been set
void SE_PhotoStudio::makePicture(VesselData *vesseldata, bool scene_prepared)
{
	Helpers::videoDriverMutex.lock();
	//set up the scene
	if (!scene_prepared)
	{
		driver->setRenderTarget(canvas);
	}

	//call in the model
	VesselSceneNode *model = new VesselSceneNode(vesseldata, smgr->getRootSceneNode(), smgr, -1);
//	model->setMaterialFlag(video::EMF_LIGHTING, true);
	setupStudioCam(model);
	//do the shoot
	driver->beginScene();
	smgr->drawAll();
	driver->endScene();

	//develop the picture
	dumpCanvasToDisk(vesseldata->className);
	//the model may leave now
	model->remove();
	
	//if the scene was prepared by somebody else, then it will also be restored by somebody else
	if (!scene_prepared)
	{
		driver->setRenderTarget(0, true, true, 0);
	}

	Helpers::videoDriverMutex.unlock();


}


//sets up the camera for the photoshoot
void SE_PhotoStudio::setupStudioCam(VesselSceneNode *model)
{
	if (studioCam == NULL)
	{
		//create camera if it doesn't exist yet
		studioCam = smgr->addCameraSceneNode();
		studioCam->bindTargetAndRotation(true);
		//use a narrow field of view to avoid large distortions
		studioCam->setFOV(camFOV * core::DEGTORAD);
	}
	
	//get the extent of the model's bounding box
	core::vector3df extent = model->getBoundingBox().getExtent();
	//calculate how high and wide the model will be in two dimensional projection when viewed from the camera.
	//this isn't entirely accurate as it doesn't take actual perceptive distortion into account, leading to the image being slightly off-center.
	f32 requiredwidth = extent.X * cos(abs(camRotation.Y) * core::DEGTORAD) + extent.Z * sin(abs(camRotation.Y) * DEGTORAD);
	f32 requiredheight = extent.Y * cos(abs(camRotation.X) * core::DEGTORAD) + extent.X * sin(abs(camRotation.X) * DEGTORAD);
	//since our images are quadratic, we're only really interested in the larger of the two. we also enlarge it a bit to have a bit of a border
	f32 largestdim = Helpers::max(requiredwidth, requiredheight) * 1.2;

	//now calculate how far away the camera needs to be to accomodate these dimensions with its given FOV
	f32 cam_radius = (largestdim / 2) / tan((camFOV / 2) * core::DEGTORAD);

	core::vector3df pos = core::vector3df(0, 0, cam_radius);
	pos.rotateXZBy(camRotation.Y);
	pos.rotateYZBy(camRotation.X);
	studioCam->setPosition(pos);

	studioCam->setTarget(vector3df(0, 0, 0));
//	smgr->setActiveCamera(studioCam);
}

//writes the contents of canvas to an image
void SE_PhotoStudio::dumpCanvasToDisk(std::string filename)
{
	std::string completefilename = Helpers::workingDirectory + "/StackEditor/Images/" + "test" + ".bmp";

	//create an IImage from the texture data
	video::IImage * image = driver->createImageFromData(
		canvas->getColorFormat(),
		canvas->getSize(),
		canvas->lock(),
		true, false);			//image gets pointer to texture data without copying. drop the image before unlocking the texture!

	driver->writeImageToFile(image, completefilename.c_str());
	image->drop();
	canvas->unlock();
}

