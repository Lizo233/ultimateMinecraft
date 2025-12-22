#pragma once
#include <string>
#include <functional>
#include <vector>
#include <iostream>
#include <fstream>
#include <time.h>
#include <filesystem>
#include <sstream>
#include <map>
#include <chrono>

//第三方库
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <tinyfiledialogs.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>



//各个常量
constexpr int DEFAULT_WIDTH = 1600;
constexpr int DEFAULT_HEIGHT = 1200;

//各个变量
double deltaTime;
double lastFrame;

std::string getTime() {
	time_t time_a;
	time(&time_a);
	char tmp[64];
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H-%M-%S", localtime(&time_a));
	return tmp;
}

std::fstream LogFile;
int logSystemInit() {

	std::string time = getTime();
	std::filesystem::create_directory("Logs");

	LogFile.open(("Logs/" + time + ".log").c_str(), std::ios::out); // 以输出模式打开文件

	if (!LogFile) {
		std::cerr << "Unable to log file!" << std::endl;
		tinyfd_messageBox("错误日志系统启动失败", "无法启动日志系统", "ok", "error", 1);
		return 1; // 文件打开失败
	}

	LogFile << "日志系统已启动\n" << std::endl; // 写入文本
	return 0;
}

void logInfo(std::string logText) {
	LogFile << "[INFO] " + logText + '\n';
}
void logWarn(std::string logText) {
	LogFile << "[WARN] " + logText + '\n';
}
void logError(std::string logText) {
	LogFile << "[ERROR] " + logText << std::endl;
}
void logFatal(std::string logText) {
	LogFile << "[FATAL] " + logText << std::endl;
}
void logPanic(std::string logText) {
	LogFile << "[PANIC] " + logText << std::endl;
}

//自定义UUID
struct UUID {
	long long int high_UUID;
	long long int low_UUID;
};

//方块的声音
class InteractSound {
	std::string something;
};

//方块的材质
class TextureCube {
	std::string something;
};

//方块的属性
using Listener = std::function<void()>;
class ATTRIBUTE {
	std::vector<Listener> listeners;
};

//函数返回值日志
//函数将会返回日志，函数在执行完毕之后会将一些结果push进retLogs vector
struct RetLog {
	int retValue;//返回值
	std::string comment;//情况报告
};
std::vector<RetLog> retLogs;//返回日志数组

//定义方块的数据
struct BlockStruct {

	long long int BlockID;
	UUID BlockUUID;//自定义UUID库
	std::string BlockNamespaceID;//命名空间ID，以后可能会用到

	ATTRIBUTE attributes;//方块属性，我们会将方块属性中的函数附加到更新监听器
	std::string BlockName;//方块的显示名称
	std::string BlockDescription;//方块的介绍
	TextureCube BlockTexture;//方块的材质，六个面
	InteractSound BlockSound;//方块交互，如摧毁、放置、挖掘的声音

	std::string comment;//注释，可能在编辑器中会用到？
};

BlockStruct BasicBlocks[1024];

void BlockDataInitial() {
	//空气的方块结构
	BasicBlocks[0].BlockID = 0;
	BasicBlocks[0].BlockName = "空气";//这个东西之后可能要考虑国际化的lang文件？

	BasicBlocks[1].BlockID = 1;
	BasicBlocks[1].BlockName = "石头";

	logInfo("方块数据初始化完毕");
}

//缓冲区大小改变回调函数
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

//鼠标滚轮回调函数
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	//camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

//为了运行，只能出此下策，注意main.cpp必须包含camera.h！
extern void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
//OpenGL初始化
GLFWwindow* OGLInitial() {
	GLFWwindow* mWindow;
	
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	mWindow = glfwCreateWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, "Ultimate Minecraft", NULL, NULL);
	if (mWindow == nullptr) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return nullptr;
	}

	glfwMakeContextCurrent(mWindow);
	glfwSetFramebufferSizeCallback(mWindow, framebuffer_size_callback);
	glfwSetCursorPosCallback(mWindow, mouse_callback);
	glfwSetScrollCallback(mWindow, scroll_callback);

	//启用捕获鼠标
	glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//加载GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "无法初始化GLAD" << std::endl;
		logError("无法初始化GLAD");
		return nullptr;
	}
	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
	LogFile << "[INFO] " << "OpenGL Version: " << glGetString(GL_VERSION) << '\n';

	logInfo("OpenGL初始化完毕");

	return mWindow;
}

//更新时间
void timeUpdate() {
	double currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
}
//清屏
void clearScreen() {
	glClearColor(0.2f * sin(glfwGetTime()), 0.3f, 0.3f * cos(glfwGetTime()), 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

//顶点属性对象
std::map<std::string, unsigned int> vaoMap;
void vaoInit() {
	glGenVertexArrays(1, &vaoMap["square"]);


	logInfo("vao初始化完成");
}

//顶点数据对象
std::map<std::string, unsigned int> vboMap;
void vboInit() {
	float squareVertices[] = {
		-1.0f,1.0f,1.0f,
		-1.0f,-1.0f,1.0f,
		1.0f,-1.0f,1.0f,
		1.0f,1.0f,1.0f,
		-1.0f,1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f,1.0f,-1.0f,
	};

	glGenBuffers(1, &vboMap["square"]);
	glBindVertexArray(vaoMap["square"]);
	glBindBuffer(GL_ARRAY_BUFFER, vboMap["square"]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), squareVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	logInfo("vbo初始化完成");
}

//顶点索引对象
std::map<std::string, unsigned int> eboMap;
void eboInit() {
	unsigned int squareIndices[] = {
		0,1,3,
		3,1,2,
		7,6,5,
		4,7,5,
		0,5,1,
		0,4,5,
		3,2,7,
		7,2,6,
		4,0,7,
		0,3,7,
		2,1,6,
		1,5,6
	};

	glGenBuffers(1, &eboMap["square"]);//创建ebo缓冲
	glBindVertexArray(vaoMap["square"]);//选择vao
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboMap["square"]);//将ebo绑定到vao上
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(squareIndices), squareIndices, GL_STATIC_DRAW);//将数据传输到显存
	glBindVertexArray(0);//解绑vao，防止出现问题
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);



	logInfo("ebo初始化完成");
}

//纹理
std::map<std::string, unsigned int> textureMap;
void textureInit() {
	

	logInfo("纹理加载完毕");
}

//键鼠输入初始化
void userInputInit() {


	logInfo("键鼠输入初始化完成");
}


