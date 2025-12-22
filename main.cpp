#include <main.h>
#include <camera.h>//镜头
#include <shader.h>//着色器

int main() {

	//日志系统
	logSystemInit();

	//方块数据初始化
	BlockDataInitial();
	
	//OpenGL初始化
	GLFWwindow* window = OGLInitial();
	if (!window) return -1;//窗口初始化失败

	//准备渲染
	vaoInit();
	vboInit();
	eboInit();
	textureInit();
	shaderInit();
	
	//键鼠输入初始化
	//初始化镜头
	

	userInputInit();
	
	//绑定透视投影矩阵
	glUniformBlockBinding(cubeShader->shaderProgramID,glGetUniformBlockIndex(cubeShader->shaderProgramID, "Matrices"), 0);
	uint32_t uboMatrices;
	glGenBuffers(1, &uboMatrices);
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	// define the range of the buffer that links to a uniform binding point
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

	// store the projection matrix (we only do this once now) (note: we're not using zoom anymore by changing the FoV)
	glm::mat4 projection = glm::perspective(45.0, (double)DEFAULT_WIDTH / DEFAULT_HEIGHT, 0.1, 100.0);
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	//OpenGL状态设置环节
	glEnable(GL_DEPTH_TEST);//开启深度测试
	glEnable(GL_CULL_FACE);//开启面剔除，注意三角形顶点必须是逆时针旋转的

	auto time = std::chrono::high_resolution_clock::now();
	
	logInfo("主循环开始");
	while (!glfwWindowShouldClose(window)) {
		//时间
		timeUpdate();
		//设置清屏
		clearScreen();

		//获取view矩阵
		glm::mat4 view = camera.getViewMatrix();
		glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		//绘画
		cubeShader->active();
		cubeShader->setUniMat4("model", glm::scale(glm::mat4(1.0),glm::vec3(0.5)));
		glBindVertexArray(vaoMap["square"]);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		//处理用户的输入
		processInput(window);
		

		//默认处理窗口消息的函数
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	logWarn("游戏正常退出");
	return 0;
}