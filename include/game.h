#include <main.h>

glm::mat4* modelMatrices;

struct WorldPos {
	long long x;
	long long y;
	long long z;
};

class Chunk {
	/*//这应该够多了吧，加mod什么的话应该也可以...吧？
	//不行，感觉还是要调色板才行
	//一个未压缩区块的大小: 16 x 16 x 256 x 2 byte = 128 KB
	//而minecraft则约为17KB一个区块。。。
	uint16_t blocks[16][266][16];//最多65536种方块，这里写的是区块中位于[x][y][z]的方块的方块ID（空气:0,石头:1 ...）
	*/
	//一个区块的大小是16x16x16格
	//未压缩时大小为 8192 bytes = 8 KB
public:
	Chunk(long long x, long long y,long long z) {
		mWorldPos.x = x;
		mWorldPos.y = y;
		mWorldPos.z = z;
	}
private:
	//各种状态
	bool mHasBlock{};
	bool mIsLoaded{};

	WorldPos mWorldPos;

	uint8_t blocks[16][16][16]{};//对应[x][y][z]位置的方块ID

public:
	bool hasBlock() const { return mHasBlock; }
	bool isLoaded() const { return mIsLoaded; }
	WorldPos getWorldPos() const { return mWorldPos; }

	uint8_t getBlockID(uint8_t x, uint8_t y, uint8_t z) const {
		
		if (x > 15 || y > 15 || z > 15) {
			std::cerr << "区块blocks数组下标溢出！\n";
			logError("区块blocks数组下标溢出！");
			std::terminate();
		}
		
		return blocks[x][y][z];
	}

	void getMatrix(glm::mat4* matrix) {
		
		matrix[0] = glm::translate(glm::mat4(1.0), glm::vec3(3.0));

	}

};

class Region {
public:
	//Region只使用WorldPos的 x,z 坐标，不使用 y 坐标
	Region(long long x, long long z) {
		mWorldPos.x = x;
		mWorldPos.z = z;
		
		for (int i = 0; i < 32; i++) {
			for (int j = 0; j < 16; j++) {
				for (int k = 0; k < 32; k++) {
					//Region大小为512x512所以其worldPos是512倍放大的，chunk大小为16x16x16所以其
					//WorldPos大小是16倍放大的（都是相对于方块的WorldPos来说）
					chunks[i][j][k] = new Chunk(i + x * 32, j, k + z * 32);
				}
			}
		}
	}

	WorldPos mWorldPos;

	Chunk *chunks[32][16][32];//512 x 512 x 512 = 134217728 格方块


};

Region* regions[4];


void initChunks() {
	//先不用其他的reigon
	//regions[0] = new Region(0, 0);
	
	logInfo("区块初始化完成");
}

//将区块数据加载到内存中
void loadChunks(const std::string& path) {
	
}

//根据内存中区块的数据决定要渲染哪些方块
/*
	renderAmount 将要渲染的方块的个数
*/
void renderChunks(int& renderAmount) {
	
	//与空气方块相邻的非空气方块渲染

	//返回要渲染的方块的个数为amount

	//neightbor update来确定？
	//以玩家为中心↑↓←→寻路来确定？？？
}
