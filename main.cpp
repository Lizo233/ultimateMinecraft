#include <main.h>
#include <camera.h>//相机
#include <shader.h>//着色器
#include <player.h>//玩家
#include <texture.h>//材质加载
#include <game.h>//游戏功能

int main(char argc,char *argv[],char *envp[]) {//也许会用到envp和argv?
	


	//std::function<int(int&, int&)> addAlias = [](int& a, int& b) {return a + b; };
	
	//loadChunks("Hello World eee");
	
	//fmt::print(fmt::bg(fmt::color::blue), "Hello World\n");

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
	//eboInit(); 暂时不使用ebo了，因为要上材质
	textureInit();
	shaderInit();
	
	//键鼠输入初始化

	userInputInit();
	
	//绑定透视和投影矩阵
	uboInit();

	//OpenGL状态设置环节
	glEnable(GL_DEPTH_TEST);//开启深度测试
	glEnable(GL_CULL_FACE);//开启面剔除，注意三角形顶点必须是逆时针旋转的

	//MSAA（感觉效果不是很明显）
	glfwWindowHint(GLFW_SAMPLES, 4);
	glEnable(GL_MULTISAMPLE);

	mainPlayer.setCamera(&camera);

	cubeShader->setUniInt("texture0", 0);

	//实例化

	unsigned int amount = 10000;
	glm::mat4* modelMatrices;
	modelMatrices = new glm::mat4[amount];
	srand(static_cast<unsigned int>(glfwGetTime()*10000));//初始化随机数种子，*10000是为了增大随机性，此时glfwGetTime∈(0,1)
	float radius = 15.0f;
	float offset = 2.50f;
	/*for (unsigned int i = 0; i < amount; i++)
	{
		glm::mat4 model = glm::mat4(1.0f);
		// 1. translation: displace along circle with 'radius' in range [-offset, offset]
		float angle = (float)i / (float)amount * 360.0f;
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float x = sin(angle) * radius + displacement;
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float y = displacement * 0.4f; // keep height of asteroid field smaller compared to width of x and z
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float z = cos(angle) * radius + displacement;
		model = glm::translate(model, glm::vec3(x, y, z));

		// 2. scale: Scale between 0.05 and 0.25f
		float scale = static_cast<float>(((rand() % 20) / 100.0 + 0.05)*3);
		model = glm::scale(model, glm::vec3(scale));

		// 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
		float rotAngle = static_cast<float>((rand() % 360));
		model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

		// 4. now add to list of matrices
		modelMatrices[i] = model;
	}*/
	for (int i = 0; i < 100; i++) {
		static glm::mat4 unitMat = glm::mat4(1.0);//这样可能会快点
		glm::mat4 model;

		for (int j = 0; j < 100; j++) {
			model = glm::translate(unitMat, glm::vec3(i - 50, -1.0, j - 50));
			modelMatrices[i * 100 + j] = model;
		}
	}

	
	// configure instanced array
	// -------------------------
	unsigned int instanceBuffer;
	glGenBuffers(1, &instanceBuffer);
	glBindVertexArray(vaoMap["cube"]);//绑定vao（其实在设置vao属性之前执行绑定vao也行，但为了防止你迷惑）
	glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);//把instanceBuffer绑定到openGL的vbo槽位上
	glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

	// set transformation matrices as an instance vertex attribute (with divisor 1)
	// note: we're cheating a little by taking the, now publicly declared, VAO of the model's mesh(es) and adding new vertexAttribPointers
	// normally you'd want to do this in a more organized fashion, but for learning purposes this will do.
	// -----------------------------------------------------------------------------------------------------------------------------------
	// set attribute pointers for matrix (4 times vec4)
	//glBindVertexArray(vaoMap["cube"]);//在这里绑定vao也是可行的
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);

	glBindVertexArray(0);//解绑vao

	logInfo("主循环开始");
	while (!glfwWindowShouldClose(window)) {
		//时间更新，deltaTime更新
		timeUpdate();
		//清屏
		clearScreen();

		//获取view矩阵
		glm::mat4 view = camera.getViewMatrix();
		glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);//绑定ubo
		//传输view矩阵到显存，并将它的偏移量设置为sizeof(glm::mat4)使得它不会覆盖projection矩阵
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);//解绑ubo

		//绘画
		cubeShader->active();
		cubeShader->setUniMat4("model", glm::scale(glm::mat4(1.0),glm::vec3(1.0)));//设置位移、旋转、缩放矩阵
		glBindVertexArray(vaoMap["cube"]);
		//glDrawArrays(GL_TRIANGLES, 0, 36);
		glDrawArraysInstanced(GL_TRIANGLES, 0, 36, amount);

		//处理用户的输入
		processInput(window);
		

		//默认处理窗口消息的函数
		glfwSwapBuffers(window);
		glfwPollEvents();
	}



	glfwTerminate();
	logWarn("游戏正常退出");
	//编译器会自动加 return 0;
}