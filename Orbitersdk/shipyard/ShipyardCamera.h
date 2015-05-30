#pragma once
using namespace core;

class ShipyardCamera
{
public:
	ShipyardCamera(vector3d<f32> pos, float radius, ICameraSceneNode* camera);
	~ShipyardCamera(void);
	void mInit(float mXPos, float mYPos, float mZPos, float mRPos);
	void setMinMaxRadius(float minRadius, float maxRadius);
	void StopRotation();		//called when the right mouse button is depressed
	void StartRotation();		//called when the right mouse button is pressed
	void StopTranslation();
	void StartTranslation();
	void UpdatePosition(float mouseX, float mouseY);
	void UpdateRadius(float wheel);
	bool IsActionInProgress();

	vector3d<f32> getTarget();
	vector3d<f32> getPosition();
private:
	float mX, mY, mZ, mR, PrevMouseX, PrevMouseY, PrevMouseZ;
	f32 Theta;
	f32 Phi;
	void rotateCam(float mouseX, float mouseY);
	void translateCam(float mouseX, float mouseY);
	float minRad, maxRad;
	float sensitivity;				//mouse sensitivity for rotation and translation, usually between 0 and 1
	ICameraSceneNode* camera_;
	bool rotation_in_progress_;
	bool translation_in_progress_;
};
