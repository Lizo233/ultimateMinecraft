#include <main.h>
#include <camera.h>//相机
#include <shader.h>//着色器
#include <player.h>//玩家
#include <texture.h>//材质加载
#include <game/game.h>//游戏功能
#include <game/render.h>//渲染


int main(char argc, char* argv[], char* envp[]) {//也许会用到envp和argv?

	//fmt::print(fmt::bg(fmt::color::blue), "Hello World\n");

	//将本地化设置为UTF-8
	std::locale::global(std::locale("en_US.UTF-8"));

	//日志系统
	logSystemInit();

	//方块数据初始化
	BlockDataInitial();

	//OpenGL初始化
	GLFWwindow* window = OGLInitial();
	if (!window) return -1;//窗口初始化失败

	//注意使用 glGet(参数) 需要在OGL初始化之后才能用，不然会引发异常

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
	//glfwWindowHint(GLFW_SAMPLES, 4);
	//glEnable(GL_MULTISAMPLE);

	mainPlayer.setCamera(&camera);

	cubeShader->setUniInt("texture0", 0);

	//世界位置矩阵（模型矩阵）初始化
	unsigned int amount = 15000;
	modelVecs = new glm::vec3[amount];

	//static glm::mat4 unitMat = glm::mat4(1.0);//这样可能会快点
	//默认将草方块铺成一个10x10的平面，位于y=-1.0处
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {
			modelVecs[i * 10 + j] = glm::vec3(0,-5,0);
		}
	}

	//游戏部分

	unsigned int vecIndex = 0;

	//区块初始化
	initChunks();

	//创建一个新的region
	regions[0] = new Region(0, 0, 0);

	LayeredNoise terraNoise(12345);


	/*for (int x=0; x < 16; ++x) {
		for (int z=0; z < 16; ++z) {
			regions[0]->generate(terraNoise, x, z);
		}
	}*/
	regions[0]->generate(terraNoise, 1, 1);

	//获取方块测试
	std::cout << "getblock: " << getBlock(-1,-1,-1) << '\n';

	//regions[0]->saveRegion("region.bin");
	//regions[0]->loadRegion("region.bin");
	//vecIndex = region.chunks[0][0][0]->getVecs(modelVecs, vecIndex);

	for (const auto region : regions) {
		break;//暂时禁用
		if (region)//仅当region!=nullptr时调用
		vecIndex = region->getVecs(modelVecs, vecIndex, amount);
	}



	std::vector<ChunkMesh*> meshRegion;
	meshDraw(meshRegion, regions[0]);
	
	//ChunkMesh mesh;
	//regions[0]->chunks[1][16][1]->blocks[13][13][13] = 1;
	//mesh.update(*regions[0]->chunks[1][16][1]);

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
		cubeShader->setUniMat4("model", glm::mat4(1.0));//设置位移、旋转、缩放矩阵

		glBindVertexArray(vaoMap["cube"]);
		//glDrawArrays(GL_TRIANGLES, 0, 36);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		for (ChunkMesh* mesh : meshRegion) {
			mesh->draw();
		}

		//printf("x:%f y:%f z:%f \n",mainPlayer.playerPos.x, mainPlayer.playerPos.y, mainPlayer.playerPos.z);

		//处理用户的输入
		processInput(window);
		
		//显示玩家朝向
		//std::cout << camera.mPlayer.Front.x << ' ' << camera.mPlayer.Front.y << ' ' << camera.mPlayer.Front.z << ' ' << '\n';

		//默认处理窗口消息的函数
		glfwSwapBuffers(window);
		glfwPollEvents();
	}



	glfwTerminate();
	logWarn("游戏正常退出");
	//编译器会自动加 return 0;
}