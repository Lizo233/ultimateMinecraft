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

	cubeShader->setInt("texture0", 0);

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

	//创建一个新的region
	regions.push_back(std::make_unique<Region>(0, 0, 0));

	//auto a = regionMap[{0, 0, 0}];

	//默认柏林噪声
	LayeredNoise terraNoise(12345);


	for (int x=0; x < 32; ++x) {
		for (int z=0; z < 32; ++z) {
			//regions[0]->generate(terraNoise, x, z);
		}
	}
	//regions[0]->generate(terraNoise);
	//regions[0]->chunks[0][0][0]->somevalues = 114514;


	//获取方块测试
	std::cout << "getblock: " << getBlock(1,1,-1) << '\n';

	//regions[0]->saveRegion("region.bin");
	//regions[0]->loadRegion("region.bin");
	//vecIndex = region.chunks[0][0][0]->getVecs(modelVecs, vecIndex);

	

	std::vector<std::unique_ptr<ChunkMesh>> meshRegion;
	meshRegion.reserve(10000);

	//meshDraw(meshRegion, regions[0]);
	
	//ChunkMesh mesh;
	//regions[0]->chunks[1][16][1]->blocks[13][13][13] = 1;
	//mesh.update(*regions[0]->chunks[1][16][1]);

	int x = 0;
	int y = 0;
	int z = 0;
	
	//渲染距离
	double renderDistance = 1024;

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
		cubeShader->setMat4("model", glm::mat4(1.0));//设置位移、旋转、缩放矩阵

		glBindVertexArray(vaoMap["cube"]);
		//glDrawArrays(GL_TRIANGLES, 0, 36);
		glDrawArrays(GL_TRIANGLES, 0, 36);



		// 因为坐标已经在 update 里加过世界偏移了，model 矩阵设为单位矩阵即可
		cubeShader->setMat4("model", glm::mat4(1.0f));
		//雾的设置
		cubeShader->setFloat("fogStart", 448.0f);
		cubeShader->setFloat("fogEnd", 512.0f);
		//雾的颜色和清屏颜色一样
		cubeShader->setVec3("fogColor", glm::vec3(0.2f * sin(glfwGetTime()), 0.3f, 0.3f * cos(glfwGetTime())));


		//动态生成区块
		dynamicGenerateChunk(mainPlayer, terraNoise, 8);


		//动态加载区块
		loadChunkMeshByDistance(meshRegion, 1024, mainPlayer);


		//遍历ChunkMesh然后调用它们的渲染函数
		for (auto& mesh : meshRegion) {
			mesh->draw();
		}
		
		

		//将距离太远的ChunkMesh设置为不显示
		for (auto& mesh : meshRegion) {
			
			Pos3D posChunkCenter = { mesh->posChunk.x * 16 + 8 ,mesh->posChunk.y * 16 + 8 ,mesh->posChunk.z * 16 + 8 };
			
			//区块中心与玩家的距离
			double distance = sqrt(pow(posChunkCenter.x - mainPlayer.playerPos.x, 2) +
				pow(posChunkCenter.y - mainPlayer.playerPos.y, 2) +
				pow(posChunkCenter.z - mainPlayer.playerPos.z, 2));

			//玩家与区块中心的位置大于渲染距离
			if ( distance > renderDistance) {
				mesh->tooFar = true;
			}

		}


		//将距离近的ChunkMesh设置为显示
		for (std::unique_ptr<ChunkMesh>& mesh : meshRegion) {
			
			Pos3D posChunkCenter = { mesh->posChunk.x * 16 + 8 ,mesh->posChunk.y * 16 + 8 ,mesh->posChunk.z * 16 + 8 };

			//区块中心与玩家的距离
			double distance = sqrt(pow(posChunkCenter.x - mainPlayer.playerPos.x, 2) +
				pow(posChunkCenter.y - mainPlayer.playerPos.y, 2) +
				pow(posChunkCenter.z - mainPlayer.playerPos.z, 2));

			//玩家与区块中心位置小于渲染距离
			if (distance < renderDistance) {
				mesh->tooFar = false;
			}
		}

		static int frames = 0;
		++frames;
		//每2000帧执行一次
		if (frames == 2000) {
			
			std::vector<Pos3D> posRegions;

			//获取各区域的位置
			for (const auto& region : regions) {
				posRegions.push_back(region->posRegion);
			}

			//当每个chunkMesh都离区域有一定距离时卸载区域

			for (const auto& mesh : meshRegion) {
				
				for (auto& pos : posRegions) {
					
					

				}

			}

		}

		//std::cout << "getblock: " << getBlock(1, 1, -1) << '\n';
		updateFPS();

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