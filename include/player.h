#pragma once
#include <main.h>

extern class Camera;
class Player {
private:
	std::string playerName;

public:
	//欧拉角
	double mYaw;
	double mPitch;
	glm::vec3 playerPos;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;

	Camera* mCamera;//该玩家对应的摄像机

	void setCamera(Camera* camera) {
		mCamera = camera;
	}
};

Player mainPlayer;//默认玩家角色