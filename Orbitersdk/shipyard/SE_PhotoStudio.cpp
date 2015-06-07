#include "Common.h"
#include "DataManager.h"
#include "VesselSceneNode.h"
#include "SE_PhotoStudio.h"


SE_PhotoStudio::SE_PhotoStudio(IrrlichtDevice *device)
{
	//create our own scenemanager and keep a pointer to the driver
	smgr = device->getSceneManager()->createNewSceneManager();
	driver = device->getVideoDriver();
	gui = device->getGUIEnvironment();
	//set default camera rotation and field of view(handpicked for decent results, but can be changed)
	camRotation = core::vector3df(-27, -26, 0);
	camFOV = 30;
	imgSize = 128;

	//ambient lighting level. might take some more fiddling
	smgr->setAmbientLight(video::SColor(0, 180, 180, 180));
	//add a white light so specular reflection can do its thing.
	studioLight = smgr->addLightSceneNode(0, core::vector3df(0, 0, 0),
		video::SColorf(0.45f, 0.45f, 0.45f));

	//create the render target
	canvas = driver->addRenderTargetTexture(core::dimension2d<u32>(imgSize, imgSize), "RTT1");
}


SE_PhotoStudio::~SE_PhotoStudio()
{
	//remove the camera from the scene
	smgr->drop();
	canvas->drop();
}


//makes a picture of a single VesselSceneNode
// vesseldata: the VesselData of the vessel to make a picture from
// scene_prepared: send true if the render target has already been set
ITexture *SE_PhotoStudio::makePicture(VesselData *vesseldata, string imagename)
{
	Helpers::videoDriverMutex.lock();
	//pop up a message that images are being created
	gui::IGUIWindow *msg = gui->addMessageBox(L"", L"StackEditor is loading some meshes for the first time and has to create images for them.\n \n Please be patient. This procedure will not be repeated at further startups.",
												true, 0);
	msg->draw();
	//switch the render target to our canvas		
	driver->setRenderTarget(canvas);

	//call in the model and place it in the center
	VesselSceneNode *model = new VesselSceneNode(vesseldata, smgr->getRootSceneNode(), smgr, -1);
	model->setPosition(model->getBoundingBox().getCenter() * -1);

	setupStudioCam(model);
	//do the shoot
	driver->beginScene(true, true, SColor(255, 0, 33, 70));
	smgr->drawAll();
	driver->endScene();

	//develop the picture
	ITexture *rettex = dumpCanvasToDisk(imagename);
	//the model may leave now
	model->remove();
	
	//switch back to default render target
	driver->setRenderTarget(0, true, true, 0);

	Helpers::videoDriverMutex.unlock();
	//remove the message
	gui->removeFocus(msg);
	msg->remove();
	return rettex;
}


//sets up the camera and light for the photoshoot
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
	f32 requiredwidth = extent.X * cos(abs(camRotation.Y) * core::DEGTORAD) + extent.Z * sin(abs(camRotation.Y) * core::DEGTORAD);
	f32 requiredheight = extent.Y * cos(abs(camRotation.X) * core::DEGTORAD) + extent.X * sin(abs(camRotation.X) * core::DEGTORAD);
	//since our images are quadratic, we're only really interested in the larger of the two. we also enlarge it a bit to have a bit of a border
	f32 largestdim = Helpers::max(requiredwidth, requiredheight) * 1.2;

	//now calculate how far away the camera needs to be to accomodate these dimensions with its given FOV
	f32 cam_radius = (largestdim / 2) / tan((camFOV / 2) * core::DEGTORAD);

	core::vector3df pos = core::vector3df(0, 0, cam_radius);
	pos.rotateXZBy(camRotation.Y);
	pos.rotateYZBy(camRotation.X);
	studioCam->setPosition(pos);

	studioCam->setTarget(core::vector3df(0, 0, 0));

	//calculate a position for the light. It wil be set at a position 200 in front of the vessel, shining down on 0,0,0 at a 45 degree angle
	studioLight->setPosition(core::vector3df(0, extent.Z + 200, extent.Z + 200));
	studioLight->setRadius(studioLight->getAbsolutePosition().getLength() * 2);
}


//writes the contents of canvas to an image and immediately returns the data as a new texture
ITexture *SE_PhotoStudio::dumpCanvasToDisk(std::string filename)
{
	std::string completefilename = Helpers::workingDirectory + "/StackEditor/Images/" + filename;

	//create an IImage from the texture data
	video::IImage * image = driver->createImageFromData(
		canvas->getColorFormat(),
		canvas->getSize(),
		canvas->lock(),
		true, false);			//image gets pointer to texture data without copying. drop the image before unlocking the texture!
	//write the image to file
	driver->writeImageToFile(image, completefilename.c_str());
	//copy the image to a new texture so we don't need to load the file immediately
	ITexture *rettex = driver->addTexture("tbxtex", image);
	image->drop();
	canvas->unlock();
	return rettex;
}

