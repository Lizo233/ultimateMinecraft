#pragma once
#include <main.h>
#include <player.h>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum class Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

class Camera {
private:
	//常量声明
	static constexpr double DEFAULT_YAW = 0.0;
	static constexpr double DEFAULT_PITCH = 0.0;
	static constexpr double DEFAULT_SPEED = 2.5;
	static constexpr double DEFAULT_SENSITIVITY = 0.1;
	static constexpr double DEFAULT_ZOOM = 45.0;
public:

	//这个镜头绑定的玩家
	Player& const mPlayer;//引用只能被初始化一次所以设置为常量

	//镜头的属性
	glm::vec3 Position;
	glm::vec3 WorldUp;

	//镜头选项
	double movementSpeed;
	double mouseSensitivity;
	double zoom;

	//速度系数
	double speedMul = 8.0;

	//获取观察矩阵
	glm::mat4 getViewMatrix() const
	{
		return glm::lookAt(mPlayer.playerPos, mPlayer.playerPos + mPlayer.Front, mPlayer.Up);
	}

	//处理键盘输入
	// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Camera_Movement direction, double deltaTime) {

		float velocity = movementSpeed * speedMul * deltaTime;
		if (direction == Camera_Movement::FORWARD)
			mPlayer.playerPos += mPlayer.Front * velocity;
		if (direction == Camera_Movement::BACKWARD)
			mPlayer.playerPos -= mPlayer.Front * velocity;
		if (direction == Camera_Movement::LEFT)
			mPlayer.playerPos -= mPlayer.Right * velocity;
		if (direction == Camera_Movement::RIGHT)
			mPlayer.playerPos += mPlayer.Right * velocity;
		if (direction == Camera_Movement::UP)
			mPlayer.playerPos -= WorldUp * velocity;
		if (direction == Camera_Movement::DOWN)
			mPlayer.playerPos += WorldUp * velocity;
	}

	//用向量初始化镜头
	Camera(Player& player,glm::vec3 position = glm::vec3(0.0, 0.0, 0.0), glm::vec3 up = glm::vec3(0.0, 1.0, 0.0), float yaw = DEFAULT_YAW, float pitch = DEFAULT_PITCH) : mPlayer(player), movementSpeed(DEFAULT_SPEED), mouseSensitivity(DEFAULT_SENSITIVITY), zoom(DEFAULT_ZOOM)
	{
		mPlayer.playerPos = position;
		mPlayer.mYaw = yaw;
		mPlayer.mPitch = pitch;

		//std::cout << mPlayer.playerPos.y << ' ' << player.playerPos.y << '\n';

		mPlayer.Front = glm::vec3(0.0, 0.0, -1.0);

		WorldUp = up;
		mPlayer.setCamera(this);
		updateCameraVectors();
	}

	//处理鼠标移动消息，画面会上下左右移动.
	void ProcessMouseMovement(double xoffset, double yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= mouseSensitivity;
		yoffset *= mouseSensitivity;

		mPlayer.mYaw += xoffset;
		mPlayer.mPitch += yoffset;

		// 使得向上的角度不会超过90°，防止画面翻转
		if (constrainPitch)
		{
			if (mPlayer.mPitch > 89.9)
				mPlayer.mPitch = 89.9;
			if (mPlayer.mPitch < -89.9)
				mPlayer.mPitch = -89.9;
		}

		// 用新的欧拉角更新向量
		updateCameraVectors();
	}

	//处理滚轮消息，使画面放大或缩小
	void processMouseScroll(double yoffset)
	{
		zoom -= yoffset;
		if (zoom < 1.0)
			zoom = 1.0;
		if (zoom > 45.0)
			zoom = 45.0;
	}

private:
	//通过更新后的欧拉角计算镜头的前方向向量
	void updateCameraVectors()
	{
		// 计算新的向前向量
		glm::vec3 front{};
		front.x = cos(glm::radians(mPlayer.mYaw)) * cos(glm::radians(mPlayer.mPitch));
		front.y = sin(glm::radians(mPlayer.mPitch));
		front.z = sin(glm::radians(mPlayer.mYaw)) * cos(glm::radians(mPlayer.mPitch));
		mPlayer.Front = glm::normalize(front);
		// 并且重新计算向上和向下的向量
		mPlayer.Right = glm::normalize(glm::cross(mPlayer.Front, WorldUp));  // 归一化向量，因为当你看向上方或下方时向量的绝对值会减小
		mPlayer.Up = glm::normalize(glm::cross(mPlayer.Right, mPlayer.Front));
	}
};

//声明默认的camera对象
Camera camera(mainPlayer,glm::vec3(0.0, 20.0, 0.0));


extern double deltaTime;
void processInput(GLFWwindow* window) {

	static bool firstTime = true;
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && firstTime) {
		mainPlayer.playerPos = glm::vec3(0, 0, 0);
		mainPlayer.Front = glm::vec3(0, 0, 1);
	}

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);//Q键关闭窗口

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera_Movement::DOWN, deltaTime);
}

//鼠标移动回调函数
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	static double lastX = DEFAULT_WIDTH / 2;
	static double lastY = DEFAULT_HEIGHT / 2;
	static bool firstMouse = true;

	double xpos = xposIn;
	double ypos = yposIn;

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	//更改移动速度
	if (action==GLFW_PRESS) {
	if (key == GLFW_KEY_EQUAL)
		camera.speedMul *= 2;
	if (key == GLFW_KEY_MINUS)
		camera.speedMul /= 2;
	}
}