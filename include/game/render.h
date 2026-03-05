#pragma once
#include <game/game.h> // 包含 GLM 和 OpenGL 头文件
#include <game/thread_pool.h>//线程池

class ChunkMesh {
private:
    // 每个顶点包含：Position(3), UV(2), FaceID/Data(1) -> 共 6 个 float
    // 这里的坐标是相对于方块原点的局部坐标

    // 顶脸 (Top) +Y: Y=0
    constexpr static float v_top[] = {
        0.0f, 0.0f,  0.0f,  0.0f, 1.0f, 4.0f, // 后左
        0.0f, 0.0f,  1.0f,  0.0f, 0.0f, 4.0f, // 前左
        1.0f, 0.0f,  1.0f,  1.0f, 0.0f, 4.0f, // 前右
        1.0f, 0.0f,  1.0f,  1.0f, 0.0f, 4.0f, // 前右
        1.0f, 0.0f,  0.0f,  1.0f, 1.0f, 4.0f, // 后右
        0.0f, 0.0f,  0.0f,  0.0f, 1.0f, 4.0f  // 后左
    };
    // 底脸 (Bottom) -Y: Y=-1
    constexpr static float v_bottom[] = {
        0.0f, -1.0f,  1.0f,  0.0f, 1.0f, 5.0f, // 前左
        0.0f, -1.0f,  0.0f,  0.0f, 0.0f, 5.0f, // 后左
        1.0f, -1.0f,  0.0f,  1.0f, 0.0f, 5.0f, // 后右
        1.0f, -1.0f,  0.0f,  1.0f, 0.0f, 5.0f, // 后右
        1.0f, -1.0f,  1.0f,  1.0f, 1.0f, 5.0f, // 前右
        0.0f, -1.0f,  1.0f,  0.0f, 1.0f, 5.0f  // 前左
    };
    // 前脸 (Front) +Z: Z=1
    constexpr static float v_front[] = {
        0.0f, -1.0f,  1.0f,  0.0f, 0.0f, 2.0f, // 左下
        1.0f, -1.0f,  1.0f,  1.0f, 0.0f, 2.0f, // 右下
        1.0f,  0.0f,  1.0f,  1.0f, 1.0f, 2.0f, // 右上
        1.0f,  0.0f,  1.0f,  1.0f, 1.0f, 2.0f, // 右上
        0.0f,  0.0f,  1.0f,  0.0f, 1.0f, 2.0f, // 左上
        0.0f, -1.0f,  1.0f,  0.0f, 0.0f, 2.0f  // 左下
    };
    // 后脸 (Back) -Z: Z=0
    constexpr static float v_back[] = {
        1.0f, -1.0f,  0.0f,  1.0f, 0.0f, 3.0f, // 右下
        0.0f, -1.0f,  0.0f,  0.0f, 0.0f, 3.0f, // 左下
        0.0f,  0.0f,  0.0f,  0.0f, 1.0f, 3.0f, // 左上
        0.0f,  0.0f,  0.0f,  0.0f, 1.0f, 3.0f, // 左上
        1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 3.0f, // 右上
        1.0f, -1.0f,  0.0f,  1.0f, 0.0f, 3.0f  // 右下
    };
    // 左脸 (Left) -X: X=0
    constexpr static float v_left[] = {
        0.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // 后下
        0.0f, -1.0f,  1.0f,  1.0f, 0.0f, 1.0f, // 前下
        0.0f,  0.0f,  1.0f,  1.0f, 1.0f, 1.0f, // 前上
        0.0f,  0.0f,  1.0f,  1.0f, 1.0f, 1.0f, // 前上
        0.0f,  0.0f,  0.0f,  0.0f, 1.0f, 1.0f, // 后上
        0.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f  // 后下
    };
    // 右脸 (Right) +X: X=1
    constexpr static float v_right[] = {
        1.0f, -1.0f,  1.0f,  0.0f, 0.0f, 0.0f, // 前下
        1.0f, -1.0f,  0.0f,  1.0f, 0.0f, 0.0f, // 后下
        1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // 后上
        1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // 后上
        1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // 前上
        1.0f, -1.0f,  1.0f,  0.0f, 0.0f, 0.0f  // 前下
    };

    unsigned int vao{}, vbo{};
    std::vector<float> meshData; // 存储所有可见面的顶点数据
    int vertexCount = 0;



public:
    bool dataReady = false;//数据是否已经准备好了

    Pos3D posChunk{};//渲染的区块的位置，用于卸载区块

    Chunk* targetChunk{};

    bool tooFar = false;

public:
    ChunkMesh() = default;

    ~ChunkMesh() {
        // 只有在主线程销毁时才删除，或者确保销毁时有上下文
        if (vao != 0) glDeleteVertexArrays(1, &vao);
        if (vbo != 0) glDeleteBuffers(1, &vbo);

        if (targetChunk) targetChunk->isMeshLoaded = false;

    }

    // 辅助函数：添加一个面的数据到 meshData，并处理世界坐标偏移
    void addFace(const float* faceVertices, float offsetX, float offsetY, float offsetZ, int blockType) {
        for (int i = 0; i < 6; ++i) { // 6个顶点
            int base = i * 6;
            // 坐标 (x, y, z)
            meshData.push_back(faceVertices[base + 0] + offsetX);
            meshData.push_back(faceVertices[base + 1] + offsetY);
            meshData.push_back(faceVertices[base + 2] + offsetZ);
            // UV (u, v)
            meshData.push_back(faceVertices[base + 3]);
            meshData.push_back(faceVertices[base + 4]);
            // 数据：这里可以将 faceID 和 blockType 合并，或者只存 blockType
            // 为了保持和你结构一致，这里存 faceVertices 里的最后一位
            meshData.push_back(faceVertices[base + 5]);
        }
        vertexCount += 6;
    }

    void update(Chunk& chunk) {

        std::shared_lock<std::shared_mutex> lock(chunk.mutexChunk,std::try_to_lock); // 加上我们之前讨论的读锁

        if (!lock.owns_lock()) return;//

        targetChunk = &chunk;
        posChunk = chunk.posChunk;

        //将目标区块状态设为渲染
        chunk.isMeshLoaded = true;

        //清除之前的数据（更新）
        meshData.clear();
        vertexCount = 0;

        long long baseX = chunk.posChunk.x * 16;
        long long baseY = chunk.posChunk.y * 16;
        long long baseZ = chunk.posChunk.z * 16;

        for (int x = 0; x < 16; ++x) {
            for (int y = 0; y < 16; ++y) {
                for (int z = 0; z < 16; ++z) {

                    int blockType = chunk.blocks[x][y][z];
                    if (blockType == 0) continue;

                    long long wx = baseX + x;
                    long long wy = baseY + y;
                    long long wz = baseZ + z;

                    // --- 右脸 (+X) ---
                    if (x < 15) {
                        if (chunk.blocks[x + 1][y][z] == 0) addFace(v_right, wx, wy, wz, blockType);
                    }
                    else {
                        if (getBlock(wx + 1, wy, wz) == 0) addFace(v_right, wx, wy, wz, blockType);
                    }

                    // --- 左脸 (-X) ---
                    if (x > 0) {
                        if (chunk.blocks[x - 1][y][z] == 0) addFace(v_left, wx, wy, wz, blockType);
                    }
                    else {
                        if (getBlock(wx - 1, wy, wz) == 0) addFace(v_left, wx, wy, wz, blockType);
                    }

                    // --- 顶脸 (+Y) ---
                    if (y < 15) {
                        if (chunk.blocks[x][y + 1][z] == 0) addFace(v_top, wx, wy, wz, blockType);
                    }
                    else {
                        if (getBlock(wx, wy + 1, wz) == 0) addFace(v_top, wx, wy, wz, blockType);
                    }

                    // --- 底脸 (-Y) ---
                    if (y > 0) {
                        if (chunk.blocks[x][y - 1][z] == 0) addFace(v_bottom, wx, wy, wz, blockType);
                    }
                    else {
                        if (getBlock(wx, wy - 1, wz) == 0) addFace(v_bottom, wx, wy, wz, blockType);
                    }

                    // --- 前脸 (+Z) ---
                    if (z < 15) {
                        if (chunk.blocks[x][y][z + 1] == 0) addFace(v_front, wx, wy, wz, blockType);
                    }
                    else {
                        if (getBlock(wx, wy, wz + 1) == 0) addFace(v_front, wx, wy, wz, blockType);
                    }

                    // --- 后脸 (-Z) ---
                    if (z > 0) {
                        if (chunk.blocks[x][y][z - 1] == 0) addFace(v_back, wx, wy, wz, blockType);
                    }
                    else {
                        if (getBlock(wx, wy, wz - 1) == 0) addFace(v_back, wx, wy, wz, blockType);
                    }
                }
            }
        }

        dataReady = true;
    }

    void upload() {
        //生成vao,vbo
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        // 上传到 GPU
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, meshData.size() * sizeof(float), meshData.data(), GL_STATIC_DRAW);

        // 设置属性指针 (x, y, z, u, v, data)
        // 0: 位置
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // 1: UV
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // 2: FaceID / TextureData
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(5 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }

    void draw() const {

        if (tooFar) return;//距离太远就返回
        if (vertexCount == 0) return;



        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glBindVertexArray(0);
    }
};


//区块处理队列
/* 未使用
//优先级，Chunk指针
using queueChunkPair = std::pair<int, Chunk*>;
//对比优先级函数
auto queueChunkCmp = [](const queueChunkPair& a, const queueChunkPair& b) {
    return a.first > b.first;
    };

std::priority_queue
<queueChunkPair, std::vector<queueChunkPair>, decltype(queueChunkCmp)> chunkQueue(queueChunkCmp);
*/

struct ChunkOffset {
    int dx, dy, dz;
    float distSq; // 存储距离平方用于排序
};

// 全局或静态的偏移列表
static std::vector<ChunkOffset> spiralOffsets;

void initSpiralOffsets(int radius) {
    spiralOffsets.clear();
    for (int i = -radius; i <= radius; ++i) {
        for (int j = -radius; j <= radius; ++j) {
            for (int k = -radius; k <= radius; ++k) {
                // 计算到原点的平方距离：d^2 = x^2 + y^2 + z^2
                float dSq = static_cast<float>(i * i + j * j + k * k);
                // 如果你想限制在球形区域内，可以加个判断
                // if (dSq <= radius * radius) 
                spiralOffsets.push_back({ i, j, k, dSq });
            }
        }
    }

    // 按距离从近到远排序
    std::sort(spiralOffsets.begin(), spiralOffsets.end(), [](const ChunkOffset& a, const ChunkOffset& b) {
        return a.distSq < b.distSq;
        });
}


//有问题，现在会报运行时错误
void loadChunkMeshByDistance(std::vector<std::unique_ptr<ChunkMesh>>& meshChunks,
    const int unloadDistance, const Player& player) {


    // --- 1. 静态资源初始化 ---
    static ThreadPool pool(std::max(1u, std::thread::hardware_concurrency() - 1));
    static std::vector<Pos3D> spiralOffsets; // 预计算的螺旋偏移列表
    static std::unordered_set<Chunk*> loadedChunk;
    static std::vector<std::unique_ptr<ChunkMesh>> buildingMeshes;

    static Pos3D lastPlayerChunkPos{};
    static int currentSpiralIndex = 0; // 当前分片扫描的进度索引

    Pos3D currentPlayerChunkPos = { (long long)player.playerPos.x >> 4, (long long)player.playerPos.y >> 4, (long long)player.playerPos.z >> 4 };


    // --- 2. 预计算螺旋偏移量 (仅运行一次) ---

    const int radius = 32; // 加载半径

    if (spiralOffsets.empty()) {
        for (int i = -radius; i <= radius; ++i) {
            for (int j = -radius; j <= radius; ++j) {
                for (int k = -radius; k <= radius; ++k) {
                    spiralOffsets.push_back({ (long long)i, (long long)j, (long long)k });
                }
            }
        }
        // 按距离平方从小到大排序
        std::sort(spiralOffsets.begin(), spiralOffsets.end(), [](const Pos3D& a, const Pos3D& b) {
            return (a.x * a.x + a.y * a.y * 0.1 + a.z * a.z) < (b.x * b.x + b.y * b.y * 0.1 + b.z * b.z);
            //y轴优先加载
            });
    }


    // --- 3. 收集并上传已完成的网格 (带配额限制) ---
    const int MAX_UPLOADS_PER_FRAME = 3;
    int uploadsDone = 0;
    for (auto it = buildingMeshes.begin(); it != buildingMeshes.end(); ) {
        if ((*it)->dataReady) {
            if (uploadsDone < MAX_UPLOADS_PER_FRAME) {
                (*it)->upload();
                meshChunks.push_back(std::move(*it));
                it = buildingMeshes.erase(it);
                uploadsDone++;
            }
            else { ++it; }
        }
        else { ++it; }
    }


    // --- 4. 分片扫描逻辑 ---
    if (currentPlayerChunkPos.x != lastPlayerChunkPos.x ||
        currentPlayerChunkPos.y != lastPlayerChunkPos.y ||
        currentPlayerChunkPos.z != lastPlayerChunkPos.z) {
        currentSpiralIndex = 0;
        lastPlayerChunkPos = currentPlayerChunkPos;
    }

    const int CHECKS_PER_FRAME = 1500;
    int checksDone = 0;

    while (checksDone < CHECKS_PER_FRAME && currentSpiralIndex < spiralOffsets.size()) {
        Pos3D offset = spiralOffsets[currentSpiralIndex++];
        checksDone++;

        long long targetX = (currentPlayerChunkPos.x + offset.x) << 4;
        long long targetY = (currentPlayerChunkPos.y + offset.y) << 4;
        long long targetZ = (currentPlayerChunkPos.z + offset.z) << 4;

        // 1. 先拿中心区块，不存在直接跳过
        auto ptrChunk = getChunk(targetX, targetY, targetZ);
        if (!ptrChunk) continue;

        // 2. 如果已经加载过了，没必要检查邻居，直接跳过
        if (loadedChunk.find(ptrChunk) != loadedChunk.end()) continue;

        // 3. 检查邻居（为了网格生成的完整性）
        // 注意：这里必须用 continue 而不是 break！
        bool neighborsReady = true;
        const int dirs[6][3] = { {16,0,0}, {-16,0,0}, {0,16,0}, {0,-16,0}, {0,0,16}, {0,0,-16} };
        for (auto& d : dirs) {
            if (getChunk(targetX + d[0], targetY + d[1], targetZ + d[2]) == nullptr) {
                neighborsReady = false;
                break; // 这里 break 只是跳出 6 方向循环
            }
        }

        if (!neighborsReady) {
            // 如果邻居没好，我们暂时不能提交这个任务。
            // 但为了防止 spiralIndex 跑得太快导致以后漏掉它，
            // 我们可以稍微减小 index，或者让它留在原地（慎用，防止死循环）
            // 简单处理：跳过，等待下一轮重置扫描。
            continue;
        }

        // 4. 提交任务
        loadedChunk.insert(ptrChunk);
        auto newMesh = std::make_unique<ChunkMesh>();
        ChunkMesh* rawPtr = newMesh.get();
        buildingMeshes.push_back(std::move(newMesh));

        pool.enqueue([rawPtr, ptrChunk]() {
            // 再次提醒：这里面的 update 绝对不能含有 OpenGL 函数（如 glGenBuffers）
            rawPtr->update(*ptrChunk);
            });

        // 限制每帧提交的新任务数，防止内存突增
        // 1024 对 buildingMeshes 来说有点大，建议设为 128 或 256
        if (buildingMeshes.size() > 1024) break;
    }


    // --- 5. 卸载逻辑 (每秒执行一次即可，没必要每帧执行) ---
    static float unloadTimer = 0;
    unloadTimer += 0.016f; // 假设 60fps
    if (unloadTimer > 2.0f) {//每120帧刷新一次
        unloadTimer = 0;
        auto tooFar = std::remove_if(meshChunks.begin(), meshChunks.end(),
            [unloadDistance, player](const std::unique_ptr<ChunkMesh>& m) {

                //玩家距离区块中心的x,y,z
                double dx = (m->posChunk.x * 16 + 8) - player.playerPos.x;
                double dy = (m->posChunk.y * 16 + 8) - player.playerPos.y;
                double dz = (m->posChunk.z * 16 + 8) - player.playerPos.z;
                if ((dx * dx + dy * dy + dz * dz) > (double)unloadDistance * unloadDistance) {
                    loadedChunk.erase(m->targetChunk);
                    return true;
                }
                return false;
            });
        meshChunks.erase(tooFar, meshChunks.end());
        printf("total chunkMeshes loaded:%lld\n", meshChunks.size());
    }
}



//动态生成区块

// 1. 全局/静态 变量，用于管理生成任务
static std::mutex regionsMutex;
static std::mutex pendingMutex;
static std::unordered_set<Pos3D, Pos3DHash> pendingGeneration; // 存储正在生成的区块唯一 ID

// 辅助函数：将坐标压缩成一个 long long ID
Pos3D getChunkID(long long x, long long y, long long z) {
    return { x,y,z };
}

void dynamicGenerateChunk(const Player& player, const LayeredNoise& noise, const int loadRadius) {
	static ThreadPool pool(std::max(1u, std::thread::hardware_concurrency() - 1));
	static std::vector<Pos3D> genOffsets;
	static int currentGenIndex = 0;
    static Pos3D lastGenPlayerPos{ 0,0,0 };

	Pos3D playerChunkPos = { (long long)player.playerPos.x >> 4, (long long)player.playerPos.y >> 4, (long long)player.playerPos.z >> 4 };

	// 2. 初始化螺旋偏移列表 (如果半径改变需要重新生成)
	if (genOffsets.empty()) {
		for (int i = -loadRadius; i <= loadRadius; ++i) {
			for (int j = -loadRadius; j <= loadRadius; ++j) {
				for (int k = -loadRadius; k <= loadRadius; ++k) {
					genOffsets.push_back({ (long long)i, (long long)j, (long long)k });
				}
			}
		}
		std::sort(genOffsets.begin(), genOffsets.end(), [](const Pos3D& a, const Pos3D& b) {
			return (a.x * a.x + a.y * a.y + a.z * a.z) < (b.x * b.x + b.y * b.y + b.z * b.z);
			});
	}

	// 如果玩家跨越区块，重置扫描
	if (playerChunkPos.x != lastGenPlayerPos.x || playerChunkPos.y != lastGenPlayerPos.y || playerChunkPos.z != lastGenPlayerPos.z) {
		currentGenIndex = 0;
		lastGenPlayerPos = playerChunkPos;
	}

	// 3. 分片扫描（每帧只检查 1000 个位置）
	const int CHECKS_PER_FRAME = 1000;
	int checksDone = 0;

	while (checksDone < CHECKS_PER_FRAME && currentGenIndex < genOffsets.size()) {
		Pos3D offset = genOffsets[currentGenIndex++];
		checksDone++;

		long long ax = (playerChunkPos.x + offset.x) << 4;
		long long ay = (playerChunkPos.y + offset.y) << 4;
		long long az = (playerChunkPos.z + offset.z) << 4;

		if (ay < 0) continue;

		// 检查是否已存在或正在生成
		Pos3D chunkID = getChunkID(ax >> 4, ay >> 4, az >> 4);

		// 关键点：快速排除已加载的区块（getChunk 内部应是线程安全的读取）
		if (getChunk(ax, ay, az)) continue;

		{
			std::lock_guard<std::mutex> lock(pendingMutex);
			if (pendingGeneration.count(chunkID)) continue;
			pendingGeneration.insert(chunkID); // 标记为正在处理
		}

		// 4. 确保 Region 存在 (主线程操作)
		Region* region = getRegionByBlock(ax, ay, az);
		if (!region) {
			int regX = ax >> 9; int regY = ay >> 9; int regZ = az >> 9;
			auto ptrRegion = std::make_unique<Region>(regX, regY, regZ);
			region = ptrRegion.get();

			std::lock_guard<std::mutex> lock(regionsMutex);
			regions.push_back(std::move(ptrRegion));
		}

		// 5. 提交耗时的生成任务到线程池
		int chkX = (ax >> 4) & 31;
		int chkZ = (az >> 4) & 31;

		pool.enqueue([region, noise, chkX, chkZ, chunkID]() {
			// 这里是后台线程执行，最耗时的噪声计算就在这里
			region->generate(noise, chkX, chkZ);

			// 生成完成后，从待办列表中移除
			std::lock_guard<std::mutex> lock(pendingMutex);
			pendingGeneration.erase(chunkID);
			});

		// 保护：如果线程池排队太多，本帧停止提交
		if (pendingGeneration.size() > 512) break;
	}
}

void dynamicUnloadChunk(const size_t unLoadDistance) {
    // 1. 声明静态线程池，确保只初始化一次
    static ThreadPool pool(std::max(1u, std::thread::hardware_concurrency() / 4)); // 卸载不需要太多线程，占 1/4 即可
    static int frames = 0;

    if (++frames < 240) return;
    frames = 0;

    // 获取玩家当前坐标快照（Pos3D）
    glm::vec3 pPos = mainPlayer.playerPos;
    const long long unLoadDistSq = unLoadDistance * unLoadDistance;

    // 2. 获取区域锁，防止主线程此时在 regions.push_back 导致崩溃
    std::lock_guard<std::mutex> regLock(regionsMutex);

    for (auto& region : regions) {
        Region* rawRegion = region.get();

        // 3. 将每个 Region 的扫描任务丢进线程池
        pool.enqueue([rawRegion, pPos, unLoadDistSq]() {

            for (int x = 0; x < 32; ++x) {
                for (int y = 0; y < 32; ++y) {
                    for (int z = 0; z < 32; ++z) {

                        // 4. RAII 锁定区块插槽
                        // 使用 try_lock 避免因为某个区块正在 build 导致卸载线程卡死
                        std::unique_lock<std::mutex> lock(rawRegion->mtxChunks[x][y][z], std::try_to_lock);
                        if (!lock.owns_lock()) continue;

                        auto& chunk = rawRegion->chunks[x][y][z];
                        if (chunk == nullptr) continue;

                        // 核心修正：距离计算
                        // chunk->posChunk 是区块单位，需要 * 16 转为世界坐标
                        long long dx = pPos.x - (chunk->posChunk.x * 16);
                        long long dy = pPos.y - (chunk->posChunk.y * 16);
                        long long dz = pPos.z - (chunk->posChunk.z * 16);
                        long long distSq = dx * dx + dy * dy + dz * dz;

                        // 满足条件则释放
                        if (distSq > unLoadDistSq && !chunk->isMeshLoaded) {
                            chunk.reset(); // 释放内存
                        }
                    }
                }
            }
            });
    }
}
