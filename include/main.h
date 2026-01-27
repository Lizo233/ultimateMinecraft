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
#include <mutex>
#include <sstream>
#include <iterator>

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
#include <fmt/core.h>
#include <fmt/color.h>
#include <zlib/zlib.h>

//各个常量
constexpr int DEFAULT_WIDTH = 1600;
constexpr int DEFAULT_HEIGHT = 1200;

//各个变量
double deltaTime;
double lastFrame;

//多线程
std::map<std::string,std::mutex> mtxMap;

//获取 2026-01-16 23-29-36 格式的时间(YYYY-MM-DD hh-mm-ss)
std::string getTime() {
	time_t time_a;
	time(&time_a);
	char tmp[64];
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H-%M-%S", localtime(&time_a));
	return tmp;
}

//日志系统启动
std::fstream LogFile;
int logSystemInit() {

	std::string time = getTime();
	std::filesystem::create_directory("Logs");

	LogFile.open(("Logs/" + time + ".log").c_str(), std::ios::out); // 以输出模式打开文件

	if (!LogFile) {
		std::cerr << "Unable to log file!" << std::endl;
		tinyfd_messageBox("日志系统启动失败", "无法启动日志系统", "ok", "error", 1);
		return 1; // 文件打开失败
	}

	//在Log文件中写入BOM
	LogFile << "\xEF\xBB\xBF" << "日志系统已启动\n" << std::endl; // 写入文本
	return 0;
}

void inline logInfo(const std::string_view logText) {
	LogFile << "[INFO] " << logText << '\n';
}
void inline logWarn(const std::string_view logText) {
	LogFile << "[WARN] " << logText << '\n';
}
void inline logError(const std::string_view logText) {
	LogFile << "[ERROR] " << logText << std::endl;
}
void inline logFatal(const std::string_view logText) {
	LogFile << "[FATAL] " << logText << std::endl;
}
void inline logPanic(const std::string_view logText) {
	LogFile << "[PANIC] " << logText << std::endl;
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
//挨算了，既然他能用就不要动了吧（滑稽）
extern void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
//OpenGL初始化
GLFWwindow* OGLInitial() {
	
	GLFWwindow* mWindow;
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);//OpenGL版本设置为 4.6
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

	//设置帧率为显示器刷新率
	glfwSwapInterval(1);

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
	glGenVertexArrays(1, &vaoMap["cube"]);//方块默认的vao


	logInfo("vao初始化完成");
}

//顶点数据对象
std::map<std::string, unsigned int> vboMap;
void vboInit() {
	//
	float cubeVertices[] = {
		// Back face
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f, // Bottom-left
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f, // top-right
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f, // bottom-right         
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f, // top-right
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f, // bottom-left
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f, // top-left
		// Front face
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, // bottom-left
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, // bottom-right
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, // top-right
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, // top-right
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f, // top-left
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, // bottom-left
		// Left face
		-0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, // top-right
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f, // top-left
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f, // bottom-left
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f, // bottom-left
		-0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, // bottom-right
		-0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, // top-right
		// Right face
		 0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f, // top-left
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f, // bottom-right
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f, // top-right         
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f, // bottom-right
		 0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f, // top-left
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, // bottom-left     
		 // Bottom face
		 -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f, // top-right
		  0.5f, -0.5f, -0.5f,  1.0f, 1.0f, -1.0f, // top-left
		  0.5f, -0.5f,  0.5f,  1.0f, 0.0f, -1.0f, // bottom-left
		  0.5f, -0.5f,  0.5f,  1.0f, 0.0f, -1.0f, // bottom-left
		 -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, -1.0f, // bottom-right
		 -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f, // top-right
		 // Top face
		 -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f, // top-left
		  0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f, // bottom-right
		  0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, // top-right     
		  0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f, // bottom-right
		 -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f, // top-left
		 -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f, // bottom-left     
	};

	glGenBuffers(1, &vboMap["cube"]);//创建VBO
	glBindVertexArray(vaoMap["cube"]);//激活VAO
	glBindBuffer(GL_ARRAY_BUFFER, vboMap["cube"]);//把vbo绑定到当前激活的vao上
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	//槽位0，位置
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);//激活0号槽
	//槽位1，纹理uv
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);//激活1号槽
	//槽位2，侧面、顶部、底部分辨
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);//激活2号槽
	glBindVertexArray(0);//解绑VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);//解绑VBO

	logInfo("vbo初始化完成");
}

//顶点索引对象
std::map<std::string, unsigned int> eboMap;
void eboInit() {

	/*float squareVertices[] = {
		-1.0f,1.0f,1.0f,//0
		-1.0f,-1.0f,1.0f,//1
		1.0f,-1.0f,1.0f,//2
		1.0f,1.0f,1.0f,//3
		-1.0f,1.0f,-1.0f,//4
		-1.0f,-1.0f,-1.0f,//5
		1.0f,-1.0f,-1.0f,//6
		1.0f,1.0f,-1.0f,//7
	};

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

	float squareVertices2[] = {
		-1.0f,1.0f,1.0f,
		-1.0f,-1.0f,1.0f,
		1.0f,-1.0f,1.0f,

		1.0f,1.0f,1.0f,
		-1.0f,-1.0f,1.0f,
		1.0f,-1.0f,1.0f,

		1.0f,1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,

		-1.0f,1.0f,-1.0f,
		1.0f,1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,

		-1.0f,1.0f,1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,1.0f,

		-1.0f,1.0f,1.0f,
		-1.0f,1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,

		1.0f,1.0f,1.0f,
		1.0f,-1.0f,1.0f,
		1.0f,1.0f,-1.0f,

		1.0f,1.0f,-1.0f,
		1.0f,-1.0f,1.0f,
		1.0f,-1.0f,-1.0f,

		-1.0f,1.0f,-1.0f,
		-1.0f,1.0f,1.0f,
		1.0f,1.0f,-1.0f,

		-1.0f,1.0f,1.0f,
		1.0f,1.0f,1.0f,
		1.0f,1.0f,-1.0f,

		1.0f,-1.0f,1.0f,
		-1.0f,-1.0f,1.0f,
		1.0f,-1.0f,-1.0f,

		-1.0f,-1.0f,1.0f,
		-1.0f,-1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
	};

	glGenBuffers(1, &eboMap["square"]);//创建ebo缓冲
	glBindVertexArray(vaoMap["square"]);//选择vao
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboMap["square"]);//将ebo绑定到vao上
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(squareIndices), squareIndices, GL_STATIC_DRAW);//将数据传输到显存
	glBindVertexArray(0);//解绑vao，防止出现问题
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);*/



	logInfo("ebo初始化完成");
}

//键鼠输入初始化
void userInputInit() {


	logInfo("键鼠输入初始化完成");
}

//zlib 压缩
// 压缩字符串
std::string compress_string(const std::string& input) {
	if (input.empty()) return {};

	// 计算最大压缩后大小
	uLong compressed_size = compressBound(static_cast<uLong>(input.size()));
	std::vector<Bytef> buffer(compressed_size);

	int result = compress(
		buffer.data(),
		&compressed_size,
		reinterpret_cast<const Bytef*>(input.data()),
		static_cast<uLong>(input.size())
	);

	if (result != Z_OK) {
		throw std::runtime_error("zlib compress failed");
	}

	return std::string(reinterpret_cast<char*>(buffer.data()), compressed_size);
}

//解压字符串
std::string decompress_string(const std::string& compressed) {
	if (compressed.empty()) return {};

	// 先获取原始大小（需要两次调用 inflate）
	z_stream strm{};
	strm.next_in = (Bytef*)compressed.data();
	strm.avail_in = static_cast<uInt>(compressed.size());

	// 初始化 inflate
	if (inflateInit(&strm) != Z_OK) {
		throw std::runtime_error("inflateInit failed");
	}

	// 先估算输出大小（或动态扩展）
	std::string output;
	const size_t chunk_size = 8192;
	output.resize(chunk_size);
	int ret;

	do {
		strm.next_out = reinterpret_cast<Bytef*>(&output[strm.total_out]);
		strm.avail_out = static_cast<uInt>(output.size() - strm.total_out);

		ret = inflate(&strm, Z_NO_FLUSH);

		if (ret == Z_STREAM_END) break;

		if (ret != Z_OK) {
			inflateEnd(&strm);
			throw std::runtime_error("inflate failed");
		}

		// 如果缓冲区不够，扩容
		if (strm.avail_out == 0) {
			output.resize(output.size() + chunk_size);
		}
	} while (true);

	output.resize(strm.total_out);
	inflateEnd(&strm);
	return output;
}