#pragma once
#include <game.h>

class ChunkMesh {
public:
	const float top[] = {
		// Top face
		 -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f, // top-left
		  0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f, // bottom-right
		  0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, // top-right     
		  0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f, // bottom-right
		 -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f, // top-left
		 -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f, // bottom-left 
	}
	const float bottom[] = {
		// Bottom face
		 -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f, // top-right
		  0.5f, -0.5f, -0.5f,  1.0f, 1.0f, -1.0f, // top-left
		  0.5f, -0.5f,  0.5f,  1.0f, 0.0f, -1.0f, // bottom-left
		  0.5f, -0.5f,  0.5f,  1.0f, 0.0f, -1.0f, // bottom-left
		 -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, -1.0f, // bottom-right
		 -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f, // top-right
	}
	const float front[] = {
		// Front face
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, // bottom-left
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, // bottom-right
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, // top-right
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, // top-right
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f, // top-left
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, // bottom-left
	}
	const float back[] = {
		// Back face
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f, // Bottom-left
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f, // top-right
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f, // bottom-right         
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f, // top-right
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f, // bottom-left
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f, // top-left
	}
	const float left[] = {
		// Left face
		-0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, // top-right
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f, // top-left
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f, // bottom-left
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f, // bottom-left
		-0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, // bottom-right
		-0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f, // top-right
	}
	const float right[] = {
		// Right face
		 0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f, // top-left
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f, // bottom-right
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f, // top-right         
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f, // bottom-right
		 0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f, // top-left
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, // bottom-left   
	}
	
	unsigned int vao;
	unsigned int vboFace;//渲染哪一平面
	unsigned int vboPos;//方块的世界坐标
	unsigned int vboTexture;//平面的纹理

	std::vector<float> dataFace;
	std::vector<float> dataPos;
	std::vector<int> dataTexture;

public:
	ChunkMesh() {
		//生成vao,vbo
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vboFace);
		glGenBuffers(1, &vboPos);
		glGenBuffers(1, &vboTexture);
	}

	~ChunkMesh() {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vboFace);
		glDeleteBuffers(1, &vboPos);
		glDeleteBuffers(1, &vboTexture);
	}

	//创建或修改ChunkMesh的数据
	void update(Chunk& chunk) {
		int blockIndex = 0;

		Pos3D baseBlock = { chunk.posChunk.x * 16 ,chunk.posChunk.y * 16 ,chunk.posChunk.z * 16 };//blocks[0][0][0]方块的位置

		//在此重写区块中哪些面要显示

		for (int ax = 0; ax < 15; ++ax)
			for (int ay = 0; ay < 15; ++ay)
				for (int az = 0; az < 15; ++az) {
					int x = baseBlock.x + ax;
					int y = baseBlock.y + ay;
					int z = baseBlock.z + az;


					getBlock(1, 1, 1);
				}

	}

	void draw() {
		
	}

};