#include <main.h>
#include <cereal/archives/binary.hpp>
#include <cereal/types/memory.hpp>

glm::vec3* modelVecs;

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

	Chunk(long long x, long long y,long long z) {
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

	uint8_t blocks[16][16][16]{};//对应[x][y][z]位置的方块ID

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

	//返回值为最后未修改的下标
	unsigned int getVecs(glm::vec3* vecs, unsigned int IndexOffset,unsigned int maxCount) const {

		if (mIsVisible == 0) return IndexOffset;//玩家看不见就直接不渲染
		if (mIsLoaded == 0) return IndexOffset;//没加载也不渲染（没加载就应该不会调用这个函数？）
		if (mHasBlock == 0) return IndexOffset;//区块里没有方块也不渲染

		unsigned int vecIndex = IndexOffset;//防止不同Chunk的矩阵之间互相覆盖

		//世界偏移量，区块在世界的哪个位置（区块坐标轴）
		glm::vec3 WorldOffset(mWorldPos.x * 16, mWorldPos.y * 16, mWorldPos.z * 16);

		int count = 0;
		for (int x = 0; x < 16; ++x) {
			for (int y = 0; y < 16; ++y) {
				for (int z = 0; z < 16; ++z) {
					if (blocks[x][y][z] == 0) continue;  // 跳过空气

					count++;

					// 检查6个方向，任意一个暴露即渲染
					bool exposed =
						(x == 0 || blocks[x - 1][y][z] == 0) ||  // 左
						(x == 15 || blocks[x + 1][y][z] == 0) ||  // 右
						(y == 0 || blocks[x][y - 1][z] == 0) ||  // 下
						(y == 15 || blocks[x][y + 1][z] == 0) ||  // 上
						(z == 0 || blocks[x][y][z - 1] == 0) ||  // 前
						(z == 15 || blocks[x][y][z + 1] == 0);    // 后

					//如果IndexOffset的过大就直接返回它
					if (exposed && (IndexOffset + vecIndex) < maxCount) {
						vecs[vecIndex++] = glm::vec3(x, y, z) + WorldOffset;
					}
				}
			}
		}
		//std::cout << "总判断量：" << count << '\n';

		//for (int i = 0; i < 16; i++) {
		//	for (int j = 0; j < 16; j++) {
		//		for (int k = 0; k < 16; k++) {
		//			if (blocks[i][j][k] != 0) {
		//				vecs[vecIndex++] = glm::vec3(i, j, k) + WorldOffset; 
		//				//std::cout << "aaa\n";
		//			}
		//			//如果在[x][y][z]处存在方块，就渲染
		//		}
		//	}
		//}

		return vecIndex;//返回最后未修改的下标
	}

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

constexpr int regionX = 32;
constexpr int regionY = 32;
constexpr int regionZ = 32;

class Region {
public:


	//Region只使用WorldPos的 x,z 坐标，不使用 y 坐标
	Region(long long x, long long z) {
		mWorldPos.x = x;
		mWorldPos.z = z;

		for (int i = 0; i < regionX; i++) {
			for (int j = 0; j < regionY; j++) {
				for (int k = 0; k < regionZ; k++) {
					//Region大小为512x512所以其worldPos是512倍放大的，chunk大小为16x16x16所以其
					//WorldPos大小是16倍放大的（都是相对于方块的WorldPos来说）
					chunks[i][j][k] = std::make_unique<Chunk>(i + x * regionX, j, k + z * regionY);
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

Region* regions[4];


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
