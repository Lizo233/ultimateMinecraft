#pragma once
#include <vector>
#include <game/game.h> // 包含 GLM 和 OpenGL 头文件

class ChunkMesh {
private:
    // 每个顶点包含：Position(3), UV(2), FaceID/Data(1) -> 共 6 个 float
    // 这里的坐标是相对于方块原点的局部坐标

    // 顶脸 (Top) +Y: Y=0
    constexpr static float v_top[] = {
        0.0f, 0.0f, -1.0f,  0.0f, 1.0f, 4.0f, // 后左
        0.0f, 0.0f,  0.0f,  0.0f, 0.0f, 4.0f, // 前左
        1.0f, 0.0f,  0.0f,  1.0f, 0.0f, 4.0f, // 前右
        1.0f, 0.0f,  0.0f,  1.0f, 0.0f, 4.0f, // 前右
        1.0f, 0.0f, -1.0f,  1.0f, 1.0f, 4.0f, // 后右
        0.0f, 0.0f, -1.0f,  0.0f, 1.0f, 4.0f  // 后左
    };
    // 底脸 (Bottom) -Y: Y=-1
    constexpr static float v_bottom[] = {
        0.0f, -1.0f,  0.0f,  0.0f, 1.0f, 5.0f, // 前左
        0.0f, -1.0f, -1.0f,  0.0f, 0.0f, 5.0f, // 后左
        1.0f, -1.0f, -1.0f,  1.0f, 0.0f, 5.0f, // 后右
        1.0f, -1.0f, -1.0f,  1.0f, 0.0f, 5.0f, // 后右
        1.0f, -1.0f,  0.0f,  1.0f, 1.0f, 5.0f, // 前右
        0.0f, -1.0f,  0.0f,  0.0f, 1.0f, 5.0f  // 前左
    };
    // 前脸 (Front) +Z: Z=0
    constexpr static float v_front[] = {
        0.0f, -1.0f,  0.0f,  0.0f, 0.0f, 2.0f, // 左下
        1.0f, -1.0f,  0.0f,  1.0f, 0.0f, 2.0f, // 右下
        1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 2.0f, // 右上
        1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 2.0f, // 右上
        0.0f,  0.0f,  0.0f,  0.0f, 1.0f, 2.0f, // 左上
        0.0f, -1.0f,  0.0f,  0.0f, 0.0f, 2.0f  // 左下
    };
    // 后脸 (Back) -Z: Z=-1
    constexpr static float v_back[] = {
        1.0f, -1.0f, -1.0f,  1.0f, 0.0f, 3.0f, // 右下
        0.0f, -1.0f, -1.0f,  0.0f, 0.0f, 3.0f, // 左下
        0.0f,  0.0f, -1.0f,  0.0f, 1.0f, 3.0f, // 左上
        0.0f,  0.0f, -1.0f,  0.0f, 1.0f, 3.0f, // 左上
        1.0f,  0.0f, -1.0f,  1.0f, 1.0f, 3.0f, // 右上
        1.0f, -1.0f, -1.0f,  1.0f, 0.0f, 3.0f  // 右下
    };
    // 左脸 (Left) -X: X=0
    constexpr static float v_left[] = {
        0.0f, -1.0f, -1.0f,  0.0f, 0.0f, 1.0f, // 后下
        0.0f, -1.0f,  0.0f,  1.0f, 0.0f, 1.0f, // 前下
        0.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // 前上
        0.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // 前上
        0.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // 后上
        0.0f, -1.0f, -1.0f,  0.0f, 0.0f, 1.0f  // 后下
    };
    // 右脸 (Right) +X: X=1
    constexpr static float v_right[] = {
        1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // 前下
        1.0f, -1.0f, -1.0f,  1.0f, 0.0f, 0.0f, // 后下
        1.0f,  0.0f, -1.0f,  1.0f, 1.0f, 0.0f, // 后上
        1.0f,  0.0f, -1.0f,  1.0f, 1.0f, 0.0f, // 后上
        1.0f,  0.0f,  0.0f,  0.0f, 1.0f, 0.0f, // 前上
        1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f  // 前下
    };

    unsigned int vao, vbo;
    std::vector<float> meshData; // 存储所有可见面的顶点数据
    int vertexCount = 0;


public:

    Pos3D posChunk{};//渲染的区块的位置，用于卸载区块

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
                    float wx = (float)(baseX + x);
                    float wy = (float)(baseY + y);
                    float wz = (float)(baseZ + z);



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
    }

    void draw() {

        if (tooFar) return;//距离太远就返回
        if (vertexCount == 0) return;

        cubeShader->active();
        // 因为坐标已经在 update 里加过世界偏移了，model 矩阵设为单位矩阵即可
        cubeShader->setUniMat4("model", glm::mat4(1.0f));

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glBindVertexArray(0);
    }
};

//给出mesh
void meshDraw(std::vector<ChunkMesh*>& mesh,Region* region) {
    for (int i = 0; i < regionX; i++) {
        for (int j = 0; j < regionY; j++) {
            for (int k = 0; k < regionZ; k++) {
                //逐个变mesh
                if (region->chunks[i][j][k] == nullptr) continue;//跳过未加载区块

                ChunkMesh* myMesh = new ChunkMesh;
                myMesh->update(*region->chunks[i][j][k]);

                mesh.push_back(myMesh);

            }
        }
    }
}