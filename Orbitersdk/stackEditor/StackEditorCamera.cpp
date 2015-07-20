//Copyright (c) 2015 Christopher Johnstone(meson800) and Benedict Haefeli(jedidia)
//The MIT License - See ../../LICENSE for more info
#include "Common.h"
#include "StackEditorCamera.h"

StackEditorCamera::StackEditorCamera(core::vector3d<f32> pos, float radius, IrrlichtDevice *device, ICameraSceneNode* camera) 
	: mX(pos.X), mY(pos.Y), mZ(pos.Z), mR(radius), camera_(camera), sensitivity(0.5), 
	Theta(90.f), Phi(90.f), minRad(10), maxRad(20000), rotation_in_progress_(false), translation_in_progress_(false)
{
	//the camera parent is also its target. When translating, only the parent will be moved, leaving camera placement to irrlicht
//	camera_->getParent()->setPosition(vector3d<f32>(0, 0, 0));
	camera_->setPosition(vector3d<f32>(mX + mR, mY, mZ));
	camera_->setTarget(camera_->getParent()->getPosition());
	device_ = device;
}

StackEditorCamera::~StackEditorCamera(void)
{
}

void StackEditorCamera::StopRotation()
{
	rotation_in_progress_ = false;
}

void StackEditorCamera::StartRotation()
{
	rotation_in_progress_ = true;
}

void StackEditorCamera::UpdatePosition(float mouseX, float mouseY, bool cntrl)
{
//	translation_in_progress_ = cntrl;
	if (rotation_in_progress_ )
	{
		rotateCam(mouseX, mouseY);
	}
	if (translation_in_progress_ || cntrl)
	{
		translateCam(mouseX, mouseY);
	}
	PrevMouseX = mouseX;
	PrevMouseY = mouseY;
}

void StackEditorCamera::StopTranslation()
{
	translation_in_progress_ = false;
}

void StackEditorCamera::StartTranslation()
{
	translation_in_progress_ = true;
}



void StackEditorCamera::rotateCam(float mouseX, float mouseY)
{

	f32 Radius = 20.f;
	float mDelta;

	mDelta = (PrevMouseX - mouseX) * sensitivity;
    Theta += mDelta; 

	mDelta = (PrevMouseY - mouseY) * sensitivity;
	Phi += mDelta;

    if (Phi < 1.f)
       Phi = 1.f;
    else if (179.f < Phi)
       Phi = 179.f; 

	f32 sinOfPhi = sinf(Phi * core::DEGTORAD);
    f32 cosOfPhi = cosf(Phi * core::DEGTORAD);

    f32 sinOfTheta = sinf(Theta * core::DEGTORAD);
    f32 cosOfTheta = cosf(Theta * core::DEGTORAD);

    core::vector3df offset;

 
    offset.X = mR * sinOfTheta * sinOfPhi;
    offset.Y = mR * cosOfPhi;
    offset.Z = mR * cosOfTheta * sinOfPhi; 

	camera_->setPosition(camera_->getTarget() + offset);
}

void StackEditorCamera::translateCam(float mouseX, float mouseY)
{
	//get the directional vector the camera is facing
	vector3d<f32> cameradir = camera_->getTarget() - camera_->getPosition();
	cameradir.Y = 0;		//no translation of the camera in the vertical!
	cameradir = cameradir.normalize();

	//calculate movement along camera view
	vector3d<f32> linearmovement = cameradir * (PrevMouseY - mouseY) * sensitivity;
	//calculate movement perpendicular to camera view
	cameradir.rotateXZBy(90);
	vector3d<f32> perpendicularmovement = cameradir * (PrevMouseX - mouseX) * sensitivity;

	//apply translation to camera and target
//	ISceneNode* parent = camera_->getParent();
//	parent->setPosition(parent->getPosition() /*+ linearmovement*/ + perpendicularmovement);
//	camera_->setTarget(parent->getPosition());
	camera_->setPosition(camera_->getAbsolutePosition() + linearmovement + perpendicularmovement);
	camera_->setTarget(camera_->getTarget() +linearmovement + perpendicularmovement);
}

void StackEditorCamera::UpdateRadius(float wheel )
{
	mR = mR - ( wheel * (mR/20) );
	if (mR < minRad)
	{
		mR = minRad;
	}
	if (mR > maxRad)
	{
		mR = maxRad;
	}

    core::vector3df offset;
	rotateCam( PrevMouseX, PrevMouseY);
}


void StackEditorCamera::setMinMaxRadius(float minRadius, float maxRadius)
{
	minRad = minRadius;
	maxRad = maxRadius;
}

vector3d<f32> StackEditorCamera::getPosition()
{
	return camera_->getPosition();
}

vector3d<f32> StackEditorCamera::getTarget()
{
	return camera_->getTarget();
}

bool StackEditorCamera::IsActionInProgress()
//returns true if the camera is currently translating or rotating
{
	return rotation_in_progress_ || translation_in_progress_;
}

matrix4 StackEditorCamera::getMatrix()
{
	return camera_->getAbsoluteTransformation();
}

vector3df StackEditorCamera::getCursorPosAtRadius(float radius)
//returns the absolute 3d position of the point under the mousecursor at distance radius from the camera
//Well, at approximately distance radius. We're checking against a plane here, not a sphere... anyways, it works decently enough.
{
	//get the view direction of the camera and calculate absolute position of radius in view direction
	camera_->updateAbsolutePosition();
	vector3df camdir = camera_->getTarget() - camera_->getAbsolutePosition();
	vector3df pos = camdir.normalize();
	pos *= radius;
	pos = camera_->getAbsolutePosition() + pos;

	//create a plane at pos facing the camera, check for mousecursor intersection, thanks for showing me how it's done in returnMouseRelativePos
	core::plane3df plane = core::plane3df(pos, camera_->getTarget() - camera_->getAbsolutePosition());
	core::line3df ray = device_->getSceneManager()->getSceneCollisionManager()->
							getRayFromScreenCoordinates(device_->getCursorControl()->getPosition());
	plane.getIntersectionWithLine(ray.start, ray.getVector(), pos);
	return pos;
}
