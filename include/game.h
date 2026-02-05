#pragma once
#include <main.h>
#include <cereal/archives/binary.hpp>
#include <cereal/types/memory.hpp>

glm::vec3* modelVecs;

class Region;

constexpr int regionX = 32;
constexpr int regionY = 32;
constexpr int regionZ = 32;

std::map<int, std::map<int, Region*>> regionMap;

struct WorldPos {
	long long x;
	long long y;
	long long z;

	template<class Archive>
	void serialize(Archive& ar) { ar(x, y, z); }
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
	Chunk() = default;

	Chunk(long long x, long long y,long long z,Region* parentRegion) {
		mWorldPos.x = x;
		mWorldPos.y = y;
		mWorldPos.z = z;
	}

//变量区
public:
	//各种状态
	bool mHasBlock{};//它有没有方块
	bool mIsLoaded{};//是否被加载
	bool mIsVisible{};//可不可见（有没有被玩家看见）
	bool mIsGenerated{};//是否被生成

	WorldPos mWorldPos;//位于世界的哪个位置（区块坐标轴）

	uint8_t blocks[16][16][16]{};//对应[x][y][z]位置的方块ID，默认初始化为0

public:
	
	bool hasBlock() const { return mHasBlock; }
	bool isLoaded() const { return mIsLoaded; }
	bool isVisible() const { return mIsVisible; }
	bool isGenerated() const { return mIsGenerated; }

	WorldPos getWorldPos() const { return mWorldPos; }

	uint8_t getBlockID(uint8_t x, uint8_t y, uint8_t z) const {
		
		if (x > 15 || y > 15 || z > 15) {
			std::cerr << "区块blocks数组下标溢出！\n";
			logError("区块blocks数组下标溢出！");
			std::terminate();
		}
		
		return blocks[x][y][z];
	}

	void initChunk() {
		mIsVisible = true;
		mIsLoaded = true;
		mHasBlock = true;//先使得区块可见

		//blocks[0][0][0] = 1;

		blocks[1][1][1] = 1;
		blocks[3][1][1] = 1;
		blocks[2][2][1] = 1;
		blocks[1][3][1] = 1;
		blocks[3][3][1] = 1;

		blocks[2][1][2] = 1;
		blocks[1][2][2] = 1;
		blocks[3][2][2] = 1;
		blocks[2][3][2] = 1;

		blocks[2][1][0] = 1;
		blocks[1][2][0] = 1;
		blocks[3][2][0] = 1;
		blocks[2][3][0] = 1;
	}

	//返回值为最后未修改的下标（vec3数组，数组偏移，最大大小）
	unsigned int getVecs(glm::vec3* vecs, unsigned int IndexOffset, unsigned int maxCount) const;

	//序列化函数模板
	template<class Archive>
	void serialize(Archive& ar) {
		ar(mHasBlock, mIsLoaded, mIsVisible,mIsGenerated,
			mWorldPos,
			cereal::binary_data(blocks, sizeof(blocks)));
		// blocks : (Qwen3-Max) 将三维数组展平为一维进行序列化（最简单方式）
	}

	void saveChunk(const std::string& path) {
		// 1. 用 cereal 序列化到内存
		std::ostringstream oss;
		cereal::BinaryOutputArchive oar(oss);
		oar(*this);
		std::string serialized = oss.str();

		// 2. 用 zlib 压缩
		std::string compressed = compress_string(serialized);

		// 3. 写入文件
		std::ofstream file(path, std::ios::binary);
		file.write(compressed.data(), compressed.size());
	}

	void loadChunk(const std::string& path) {
		// 1. 读取压缩文件
		std::ifstream file(path, std::ios::binary);

		std::string compressed{
			(std::istreambuf_iterator<char>(file)),
			std::istreambuf_iterator<char>()
		};

		// 2. 解压
		std::string serialized = decompress_string(compressed);

		// 3. 从内存反序列化
		std::istringstream iss(serialized);
		cereal::BinaryInputArchive iarchive(iss);
		iarchive(*this);
	}

	void generate(long long seed) {
		//柏林算法生成



	}
};

class Region {
public:


	//Region只使用WorldPos的 x,z 坐标，不使用 y 坐标
	Region(long long x, long long z) {
		mWorldPos.x = x;
		mWorldPos.z = z;

		regionMap[x][z] = this;

		for (int i = 0; i < regionX; i++) {
			for (int j = 0; j < regionY; j++) {
				for (int k = 0; k < regionZ; k++) {
					//Region大小为512x512所以其worldPos是512倍放大的，chunk大小为16x16x16所以其
					//WorldPos大小是16倍放大的（都是相对于方块的WorldPos来说）
					chunks[i][j][k] = std::make_unique<Chunk>(i + x * regionX, j, k + z * regionY, this);
				}
			}
		}
	}

	WorldPos mWorldPos;

	std::unique_ptr<Chunk> chunks[regionX][regionY][regionZ];//512 x 256 x 512 = 16777216 格方块

	//序列化函数模板
	template<class Archive>
	void serialize(Archive& ar) {
		ar(mWorldPos);

		for (int i = 0; i < regionX; i++) {
			for (int j = 0; j < regionY; j++) {
				for (int k = 0; k < regionZ; k++) {
					//逐个序列化
					ar(chunks[i][j][k]);
				}
			}
		}

	}

	void saveRegion(const std::string& path) {
		// 1. 用 cereal 序列化到内存
		std::ostringstream oss;
		cereal::BinaryOutputArchive oar(oss);
		oar(*this);
		std::string serialized = oss.str();

		// 2. 用 zlib 压缩
		std::string compressed = compress_string(serialized);
		std::cout << "serialized的大小：" << serialized.size() << '\n';

		// 3. 写入文件
		std::ofstream file(path, std::ios::binary);
		file.write(compressed.data(), compressed.size());
	}

	void loadRegion(const std::string& path) {
		// 1. 读取压缩文件
		std::ifstream file(path, std::ios::binary);

		std::string compressed{
			(std::istreambuf_iterator<char>(file)),
			std::istreambuf_iterator<char>()
		};

		// 2. 解压
		std::string serialized = decompress_string(compressed);
		std::cout << "serialized的大小：" << serialized.size() << '\n';

		// 3. 从内存反序列化
		std::istringstream iss(serialized);
		cereal::BinaryInputArchive iarchive(iss);
		iarchive(*this);

		//将自己加入世界的regionMap
		regionMap[mWorldPos.x][mWorldPos.z] = this;
	}

	void unload() {

		//将自己从worldMap中移除
		regionMap[mWorldPos.x][mWorldPos.z] = nullptr;
	}

	unsigned int getVecs(glm::vec3* vecs, unsigned int IndexOffset,unsigned int maxCount) const {

		for (int i = 0; i < regionX; i++) {
			for (int j = 0; j < regionY; j++) {
				for (int k = 0; k < regionZ; k++) {
					//逐个getVecs
					IndexOffset = chunks[i][j][k]->getVecs(vecs, IndexOffset, maxCount);
				}
			}
		}
		return IndexOffset;
	}
};

Region* regions[9];


void initChunks() {
	//先不用其他的reigon
	//regions[0] = new Region(0, 0);
	
	logInfo("区块初始化完成");
}

//将区块数据加载到内存中
void loadChunks(const std::string path) {
	
}

void saveChunks(const std::string path) {
	
}

//根据内存中区块的数据决定要渲染哪些方块
/*
	maxRenderAmount 最大方块渲染数量
*/
void renderChunks(int maxRenderAmount) {
	
	//与空气方块相邻的非空气方块渲染

	//返回要渲染的方块的个数为amount
	
	//neightbor update来确定？
	//以玩家为中心↑↓←→寻路来确定？？？
}

unsigned int Chunk::getVecs(glm::vec3* vecs, unsigned int IndexOffset, unsigned int maxCount) const {

	if (mIsVisible == 0) return IndexOffset;//玩家看不见就直接不渲染
	if (mIsLoaded == 0) return IndexOffset;//没加载也不渲染（没加载就应该不会调用这个函数？）
	if (mHasBlock == 0) return IndexOffset;//区块里没有方块也不渲染

	unsigned int vecIndex = IndexOffset;//防止不同Chunk的矩阵之间互相覆盖

	//世界偏移量，区块在世界的哪个位置（区块坐标轴）
	glm::vec3 WorldOffset(mWorldPos.x * 16, mWorldPos.y * 16, mWorldPos.z * 16);


	for (int x = 0; x < 16; ++x) {
		for (int y = 0; y < 16; ++y) {
			for (int z = 0; z < 16; ++z) {
				if (blocks[x][y][z] == 0) continue;  // 跳过空气

				bool exposed = false;

				//检查6个方向，任意一个暴露即渲染
				if (x == 0 || x == 15 ||
					y == 0 || y == 15 ||
					z == 0 || z == 15) {
					//检测的方块位于区块边界

					//如果位于region边缘，或邻居区块是空的就暴露

					//是否处在region边缘
					if (mWorldPos.x % regionX == 0 || mWorldPos.y % regionY == 0 || mWorldPos.z % regionZ == 0 ||
						mWorldPos.x % regionX == 31 || mWorldPos.y % regionY == 31 || mWorldPos.z % regionZ == 31)
					{
						exposed = true;
					}
					else {
						//exposed = 1;
						//邻居区块检测
						//检测该方块与邻居区块接触的方块是否为空气

						//获取自身所在region的指针
						Region* myRegion = regionMap
							[std::floor((double)mWorldPos.x / regionX)] // 区块坐标轴向下取整
							[std::floor((double)mWorldPos.z / regionZ)];

						if (myRegion == nullptr) {
							std::cout << "[ERROR] myRegion指针为nullptr，从regionMap中获取的指针不可用" << '\n';
							logError("myRegion指针为nullptr，从regionMap中获取的指针不可用");
							std::terminate();//终止程序
						}


						//卧槽我tm的面对的原来是正x轴？
						if (z == 0) {
							exposed = myRegion->chunks[mWorldPos.x % regionX][mWorldPos.y][mWorldPos.z % regionZ - 1]->blocks[x][y][15] == 0;
						}

					}
				}
				else {
					//检测方块在区块内部

					exposed =
						(blocks[x - 1][y][z] == 0) ||  // 左
						(blocks[x + 1][y][z] == 0) ||  // 右
						(blocks[x][y - 1][z] == 0) ||  // 下
						(blocks[x][y + 1][z] == 0) ||  // 上
						(blocks[x][y][z - 1] == 0) ||  // 前
						(blocks[x][y][z + 1] == 0);    // 后
				}
				if (exposed == false)
					continue;//没暴露就直接下一个循环不做判断

				//如果 vecIndex 过大就直接返回它
				if (vecIndex < maxCount) {
					vecs[vecIndex++] = glm::vec3(x, y, z) + WorldOffset;
				}
				else {
					return vecIndex;//此时vecIndex应等于maxCount
				}
			}
		}
	}
	//std::cout << "总判断量：" << count << '\n';

	return vecIndex;//返回最后未修改的下标
}
