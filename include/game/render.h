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

    unsigned int vao, vbo;
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

        targetChunk = &chunk;

        posChunk = chunk.posChunk;

        meshData.clear();
        vertexCount = 0;

        // 基础世界坐标（注意：通常区块大小是 16x16x16）
        long long baseX = chunk.posChunk.x * 16;
        long long baseY = chunk.posChunk.y * 16;
        long long baseZ = chunk.posChunk.z * 16;

        for (int x = 0; x < 16; ++x) {
            for (int y = 0; y < 16; ++y) {
                for (int z = 0; z < 16; ++z) {


                    int blockType = chunk.blocks[x][y][z];
                    if (blockType == 0) continue; // 空气跳过


                    // 当前方块的世界坐标
                    long long wx = baseX + x;
                    long long wy = baseY + y;
                    long long wz = baseZ + z;



                    // 检查 6 个方向的邻居（使用世界坐标查询）
                    if (getBlock(wx + 1, wy, wz) == 0) addFace(v_right, wx, wy, wz, blockType);
                    if (getBlock(wx - 1, wy, wz) == 0) addFace(v_left, wx, wy, wz, blockType);
                    if (getBlock(wx, wy + 1, wz) == 0) addFace(v_top, wx, wy, wz, blockType);
                    if (getBlock(wx, wy - 1, wz) == 0) addFace(v_bottom, wx, wy, wz, blockType);
                    if (getBlock(wx, wy, wz + 1) == 0) addFace(v_front, wx, wy, wz, blockType);
                    if (getBlock(wx, wy, wz - 1) == 0) addFace(v_back, wx, wy, wz, blockType);
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

//优先级，Chunk指针
using queueChunkPair = std::pair<int, Chunk*>;
//对比优先级函数
auto queueChunkCmp = [](const queueChunkPair& a, const queueChunkPair& b) {
    return a.first > b.first;
    };

std::priority_queue
<queueChunkPair, std::vector<queueChunkPair>, decltype(queueChunkCmp)> chunkQueue(queueChunkCmp);


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
    // 如果玩家跨越了区块，重置扫描进度，从身边重新开始
    if (currentPlayerChunkPos.x != lastPlayerChunkPos.x ||
        currentPlayerChunkPos.y != lastPlayerChunkPos.y ||
        currentPlayerChunkPos.z != lastPlayerChunkPos.z) {
        currentSpiralIndex = 0;
        lastPlayerChunkPos = currentPlayerChunkPos;
    }

    // 每帧只检查 1500 个位置，防止半径 16 (3.5万个位置) 导致的卡顿
    const int CHECKS_PER_FRAME = 1500;
    int checksDone = 0;

    while (checksDone < CHECKS_PER_FRAME && currentSpiralIndex < spiralOffsets.size()) {
        Pos3D offset = spiralOffsets[currentSpiralIndex++];
        checksDone++;

        // 计算目标区块的绝对世界坐标
        long long targetX = (currentPlayerChunkPos.x + offset.x) << 4;
        long long targetY = (currentPlayerChunkPos.y + offset.y) << 4;
        long long targetZ = (currentPlayerChunkPos.z + offset.z) << 4;

        auto ptrChunk = getChunk(targetX, targetY, targetZ);

        //printf("building queue:%d\n",buildingMeshes.size());
        
        //ptrChunk存在，且不在loadedChunk中
        if (ptrChunk && loadedChunk.find(ptrChunk) == loadedChunk.end()) {
            // 提交新任务
            loadedChunk.insert(ptrChunk);
            auto newMesh = std::make_unique<ChunkMesh>();
            ChunkMesh* rawPtr = newMesh.get();
            buildingMeshes.push_back(std::move(newMesh));

            pool.enqueue([rawPtr, ptrChunk]() {
                rawPtr->update(*ptrChunk);
                });

            // 如果线程池队列太长，暂时停止提交，保护内存
            //感觉之后把这些字面数值改成全局constexpr int比较好
            if (buildingMeshes.size() > 1024) break;
        }
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
    }
}