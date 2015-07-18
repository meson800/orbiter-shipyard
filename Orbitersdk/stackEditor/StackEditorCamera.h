#pragma once
using namespace core;

class StackEditorCamera
{
public:
	StackEditorCamera(vector3d<f32> pos, float radius, IrrlichtDevice *device, ICameraSceneNode* camera);
	~StackEditorCamera(void);
	void mInit(float mXPos, float mYPos, float mZPos, float mRPos);
	void setMinMaxRadius(float minRadius, float maxRadius);
	void StopRotation();		//called when the right mouse button is depressed
	void StartRotation();		//called when the right mouse button is pressed
	void StopTranslation();
	void StartTranslation();
	void UpdatePosition(float mouseX, float mouseY, bool cntrl = false);
	void UpdateRadius(float wheel);
	bool IsActionInProgress();
	matrix4 getMatrix();
	vector3df getCursorPosAtRadius(float radius);

	vector3d<f32> getTarget();
	vector3d<f32> getPosition();
private:
	float mX, mY, mZ, mR, PrevMouseX, PrevMouseY, PrevMouseZ;
	f32 Theta;
	f32 Phi;
	IrrlichtDevice *device_;
	void rotateCam(float mouseX, float mouseY);
	void translateCam(float mouseX, float mouseY);
	float minRad, maxRad;
	float sensitivity;				//mouse sensitivity for rotation and translation, usually between 0 and 1
	ICameraSceneNode* camera_;
	bool rotation_in_progress_;
	bool translation_in_progress_;
};
