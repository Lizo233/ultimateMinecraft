#pragma once
#include <main.h>

//learnOpenGL CN 上的loadTexture
unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		//注意这里默认绑到了GL_TEXTURE0上面，因为TEXTURE0是默认激活的
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		//各向异性
		//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 16.0f);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		logError((std::string)"无法加载材质，路径：" + path);
		stbi_image_free(data);
	}
	glBindTexture(GL_TEXTURE_2D,0);//解绑

	glActiveTexture(GL_TEXTURE0);      // 激活纹理单元 0
	glBindTexture(GL_TEXTURE_2D, textureID);  // 将纹理绑定到该单元的 2D 目标

	return textureID;
}

//纹理
std::map<std::string, unsigned int> textureMap;
GLuint textureArray;
void textureInit() {

	//让stbi上下翻转图片
	stbi_set_flip_vertically_on_load(true);
	//草方块
	textureMap["grassBlock"] = loadTexture("./assets/grass-block.png");

	// 纹理数组参数
	const int TEXTURE_SIZE = 16;      // 宽高均为16像素
	const int LAYER_COUNT = 3;        // 3层

	// 三个文件名
	const char* textureFiles[LAYER_COUNT] = {//textureArray编号
		"assets/top.png",//编号0
		"assets/side.png",//编号1
		"assets/bottom.png"//编号1
	};

	//纹理数组
	glGenTextures(1, &textureArray);
	glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);
	
	// 分配不可变存储：5级mipmap，内部格式RGBA8，宽高16，3层
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 5, GL_RGBA8, TEXTURE_SIZE, TEXTURE_SIZE, LAYER_COUNT);
	

	// 设置纹理参数
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 4); // 0 ~ 4

	// 加载每一层
	stbi_set_flip_vertically_on_load(true); // 使图片原点位于左下角，与OpenGL一致
	int width, height, channels;
	for (int layer = 0; layer < LAYER_COUNT; ++layer) {
		// 强制加载为RGBA（4通道）
		unsigned char* data = stbi_load(textureFiles[layer], &width, &height, &channels, 4);
		if (!data) {
			std::cerr << "Failed to load texture: " << textureFiles[layer] << std::endl;
			continue;
		}
		// 检查尺寸
		if (width != TEXTURE_SIZE || height != TEXTURE_SIZE) {
			std::cerr << "Texture " << textureFiles[layer] << " size is not 16x16!" << std::endl;
		}
		// 上传到对应层
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
			0,                   // mipmap等级
			0, 0, layer,        // xoffset, yoffset, zoffset (层索引)
			width, height, 1,   // 宽度，高度，深度（始终为1）
			GL_RGBA,           // 上传数据的格式（与stbi_load指定的4通道对应）
			GL_UNSIGNED_BYTE,   // 数据类型
			data);
		stbi_image_free(data);
	}
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);



	logInfo("纹理加载完毕");
}
