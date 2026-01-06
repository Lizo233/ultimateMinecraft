#pragma once
#include <main.h>

extern class Camera;
class Player {
private:
	std::string playerName;

public:
	//Å·À­½Ç
	double mYaw;
	double mPitch;
	glm::vec3 playerPos;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;

	Camera* mCamera;

	void setCamera(Camera* camera) {
		mCamera = camera;
	}
};

Player mainPlayer;