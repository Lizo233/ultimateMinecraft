#pragma once
#include <game/game.h> // 包含 GLM 和 OpenGL 头文件

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
    bool updated = false;


public:

    Pos3D posChunk{};//渲染的区块的位置，用于卸载区块

    Chunk* targetChunk{};

    bool tooFar = false;

public:
    ChunkMesh() {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
    }

    ~ChunkMesh() {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
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

        if (updated) return;//如果已更新则之间返回

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

        updated = true;
    }

    void draw() const {

        if (tooFar) return;//距离太远就返回
        if (vertexCount == 0) return;

        cubeShader->active();
        // 因为坐标已经在 update 里加过世界偏移了，model 矩阵设为单位矩阵即可
        cubeShader->setMat4("model", glm::mat4(1.0f));

        //雾的设置
        cubeShader->setFloat("fogStart", 448.0f);
        cubeShader->setFloat("fogEnd", 512.0f);
        //雾的颜色和清屏颜色一样
        cubeShader->setVec3("fogColor", glm::vec3(0.2f * sin(glfwGetTime()), 0.3f, 0.3f * cos(glfwGetTime())));


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

void loadChunkMeshByDistance(std::vector<std::unique_ptr<ChunkMesh>>& meshChunks,
    const int unloadDistance, const Player& player) {

    static Pos3D pos{};

    long long x = player.playerPos.x;
    long long y = player.playerPos.y;
    long long z = player.playerPos.z;

    if (x == pos.x && y == pos.y && z == pos.z) {
        return;//玩家未移动
    }
    if (x >> 4 == pos.x >> 4 && y >> 4 == pos.y >> 4 && z >> 4 == pos.z >> 4) {
        return;//玩家移动但未离开区块
    }

    pos = { x,y,z };

    static std::set<Chunk*> loadedChunk;

    //meshChunks.clear();


    //获取周围7x7x7的区块，作为最高加载优先级

    const int loadRadius = 3;

    for (int i = -loadRadius; i <= loadRadius; ++i) {
        for (int j = -loadRadius; j <= loadRadius; ++j) {
            for (int k = -loadRadius; k <= loadRadius; ++k) {
                auto ptrChunk = getChunk(x + i * 16, y + j * 16, z + k * 16);

                if (ptrChunk == nullptr) continue;//跳过未加载区块
                //跳过处在meshChunks中的区块
                if (loadedChunk.find(ptrChunk) != loadedChunk.end()) continue;

                //将已加载的区块加入loadedChunk
                loadedChunk.insert(ptrChunk);

                std::unique_ptr<ChunkMesh> mesh = std::make_unique<ChunkMesh>();//创建ChunkMesh对象
                mesh->update(*ptrChunk);//更新它

                meshChunks.push_back(std::move(mesh));//移动到meshRegion里
            }
        }
    }


    auto tooFar = std::remove_if(meshChunks.begin(), meshChunks.end(),
        [unloadDistance,player](const std::unique_ptr<ChunkMesh>& ptr) {
            
            Pos3D posChunkCenter = { ptr->posChunk.x * 16 + 8 ,ptr->posChunk.y * 16 + 8 ,ptr->posChunk.z * 16 + 8 };

            //区块中心与玩家的距离
            double distance = sqrt(pow(posChunkCenter.x - player.playerPos.x, 2) +
                pow(posChunkCenter.y - player.playerPos.y, 2) +
                pow(posChunkCenter.z - player.playerPos.z, 2));

            if (distance > unloadDistance) {
                loadedChunk.erase(ptr->targetChunk);
                return true;
            }
            return false;

        }
    );
    meshChunks.erase(tooFar, meshChunks.end());


    printf("meshVectorsize: %d\n", meshChunks.size());

}