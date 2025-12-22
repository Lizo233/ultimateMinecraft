#pragma once
#include <main.h>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
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
	static constexpr double YAW = -90.0;
	static constexpr double PITCH = 0.0;
	static constexpr double SPEED = 2.5;
	static constexpr double SENSITIVITY = 0.1;
	static constexpr double ZOOM = 45.0;
public:



	//镜头的属性
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	//欧拉角
	double mYaw;
	double mPitch;
	//镜头选项
	double movementSpeed;
	double mouseSensitivity;
	double zoom;

	//获取观察矩阵
	glm::mat4 getViewMatrix() const
	{
		return glm::lookAt(Position, Position + Front, Up);
	}

	//处理键盘输入
	// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Camera_Movement direction, double deltaTime)
	{
		float velocity = movementSpeed * deltaTime;
		if (direction == FORWARD)
			Position += Front * velocity;
		if (direction == BACKWARD)
			Position -= Front * velocity;
		if (direction == LEFT)
			Position -= Right * velocity;
		if (direction == RIGHT)
			Position += Right * velocity;
		if (direction == UP)
			Position -= WorldUp * velocity;
		if (direction == DOWN)
			Position += WorldUp * velocity;
	}

	//用向量初始化镜头
	Camera(glm::vec3 position = glm::vec3(0.0, 0.0, 0.0), glm::vec3 up = glm::vec3(0.0, 1.0, 0.0), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0)), movementSpeed(SPEED), mouseSensitivity(SENSITIVITY), zoom(ZOOM)
	{
		Position = position;
		WorldUp = up;
		mYaw = yaw;
		mPitch = pitch;
		updateCameraVectors();
	}

	//处理鼠标移动消息，画面会上下左右移动.
	void ProcessMouseMovement(double xoffset, double yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= mouseSensitivity;
		yoffset *= mouseSensitivity;

		mYaw += xoffset;
		mPitch += yoffset;

		// 使得向上的角度不会超过90°，防止画面翻转
		if (constrainPitch)
		{
			if (mPitch > 89.9)
				mPitch = 89.9;
			if (mPitch < -89.9)
				mPitch = -89.9;
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
		front.x = cos(glm::radians(mYaw)) * cos(glm::radians(mPitch));
		front.y = sin(glm::radians(mPitch));
		front.z = sin(glm::radians(mYaw)) * cos(glm::radians(mPitch));
		Front = glm::normalize(front);
		// 并且重新计算向上和向下的向量
		Right = glm::normalize(glm::cross(Front, WorldUp));  // 归一化向量，因为当你看向上方或下方时向量的绝对值会减小
		Up = glm::normalize(glm::cross(Right, Front));
	}
};

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
extern double deltaTime;
void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);//Q键关闭窗口

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);
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

