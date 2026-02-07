#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <random>
#include <cmath>
#include <numeric>

class PerlinNoise {
private:
    std::vector<int> permutation;

    static double fade(double t) {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }

    static double lerp(double a, double b, double t) {
        return a + t * (b - a);
    }

    double gradDot(int hash, double x, double y, double z) const {
        int h = hash & 15;
        double u = h < 8 ? x : y;
        double v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
        return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
    }

public:
    PerlinNoise(unsigned int seed = 0) {
        permutation.resize(256);
        std::iota(permutation.begin(), permutation.end(), 0);

        std::default_random_engine engine(seed);
        std::shuffle(permutation.begin(), permutation.end(), engine);

        permutation.insert(permutation.end(), permutation.begin(), permutation.end());
    }

    double noise(double x, double y, double z = 0.0) const {
        int X = static_cast<int>(std::floor(x)) & 255;
        int Y = static_cast<int>(std::floor(y)) & 255;
        int Z = static_cast<int>(std::floor(z)) & 255;

        x -= std::floor(x);
        y -= std::floor(y);
        z -= std::floor(z);

        double u = fade(x);
        double v = fade(y);
        double w = fade(z);

        int A = permutation[X] + Y;
        int B = permutation[X + 1] + Y;
        int AA = permutation[A] + Z;
        int AB = permutation[A + 1] + Z;
        int BA = permutation[B] + Z;
        int BB = permutation[B + 1] + Z;

        double contrib000 = gradDot(permutation[AA], x, y, z);
        double contrib100 = gradDot(permutation[BA], x - 1, y, z);
        double contrib010 = gradDot(permutation[AB], x, y - 1, z);
        double contrib110 = gradDot(permutation[BB], x - 1, y - 1, z);

        double contrib001 = gradDot(permutation[AA + 1], x, y, z - 1);
        double contrib101 = gradDot(permutation[BA + 1], x - 1, y, z - 1);
        double contrib011 = gradDot(permutation[AB + 1], x, y - 1, z - 1);
        double contrib111 = gradDot(permutation[BB + 1], x - 1, y - 1, z - 1);

        double x1 = lerp(contrib000, contrib100, u);
        double x2 = lerp(contrib010, contrib110, u);
        double y1 = lerp(x1, x2, v);

        double x3 = lerp(contrib001, contrib101, u);
        double x4 = lerp(contrib011, contrib111, u);
        double y2 = lerp(x3, x4, v);

        return lerp(y1, y2, w);
    }
};

// 分层噪声（分形布朗运动）类
class LayeredNoise {
private:
    PerlinNoise perlin;

public:
    LayeredNoise(unsigned int seed = 0) : perlin(seed) {}

    // 生成5层噪声叠加
    double mountainNoise(double x, double y, double baseScale = 0.01) const {
        // 第1层：基础山脉形状
        double value = perlin.noise(x * baseScale, y * baseScale) * 1.0;

        // 第2层：中等细节
        //value += perlin.noise(x * baseScale * 2.0, y * baseScale * 2.0) * 0.5;

        // 第3层：小细节
        //value += perlin.noise(x * baseScale * 4.0, y * baseScale * 4.0) * 0.25; 

        // 第4层：精细纹理
        //value += perlin.noise(x * baseScale * 8.0, y * baseScale * 8.0) * 0.125;

        // 第5层：微小细节
        //value += perlin.noise(x * baseScale * 16.0, y * baseScale * 16.0) * 0.0625;

        double amplify = 2 - 0.0625;

        //归一化到[0,1]
        return (value + amplify) / (amplify * 2);
    }

    // 带参数控制的5层噪声
    double layeredNoise(double x, double y,
        double baseScale = 0.01,
        double persistence = 0.5,    // 振幅衰减系数
        int octaves = 5) const {

        double value = 0.0;
        double amplitude = 1.0;
        double frequency = 1.0;
        double maxAmplitude = 0.0;  // 用于归一化

        for (int i = 0; i < octaves; i++) {
            value += perlin.noise(x * baseScale * frequency,
                y * baseScale * frequency) * amplitude;
            maxAmplitude += amplitude;
            amplitude *= persistence;
            frequency *= 2.0;
        }

        // 归一化到[-1,1]范围
        return value / maxAmplitude;
    }

    // 生成更有趣的地形，添加山脊和山谷
    double terrainNoise(double x, double y, double scale = 0.01) const {
        // 基础地形
        double terrain = layeredNoise(x, y, scale, 0.5, 5);

        // 添加一些山脊效果（使用绝对值）
        double ridges = std::abs(perlin.noise(x * scale * 1.5, y * scale * 1.5) * 0.4);

        // 结合地形和山脊
        double combined = terrain * 0.7 + ridges * 0.3;

        // 添加一些山谷（使用第二个噪声层）
        double valleys = (perlin.noise(x * scale * 0.5, y * scale * 0.5) + 1.0) * 0.5;
        valleys = std::pow(valleys, 2.0); // 使山谷更平缓

        return combined * valleys;
    }

    // 生成岛屿/海岸线效果
    double islandNoise(double x, double y, double scale = 0.01) const {
        // 基础地形
        double base = layeredNoise(x, y, scale, 0.5, 5);

        // 创建圆形衰减（模拟岛屿）
        double centerX = 0.5;
        double centerY = 0.5;
        double distX = (x * scale - centerX);
        double distY = (y * scale - centerY);
        double distance = std::sqrt(distX * distX + distY * distY);

        // 岛屿形状：中心高，边缘低
        double islandShape = 1.0 - std::clamp(distance * 2.0, 0.0, 1.0);
        islandShape = std::pow(islandShape, 1.5); // 调整形状

        // 结合基础地形和岛屿形状
        return base * islandShape;
    }
};

// 工具函数
inline double clamp(double value, double min, double max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// 将噪声值转换为颜色值
inline int noiseToColor(double noiseValue, bool normalize = true) {
    if (normalize) {
        // 归一化到[0,1]然后映射到[0,255]
        noiseValue = clamp((noiseValue + 1.0) * 0.5, 0.0, 1.0);
    }
    else {
        // 直接使用clamp确保在[0,1]范围内
        noiseValue = clamp(noiseValue, 0.0, 1.0);
    }

    return static_cast<int>(noiseValue * 255);
}

// 生成灰度PPM图像
void savePPM(const std::string& filename,
    const std::vector<std::vector<double>>& heightmap,
    int width, int height) {

    std::ofstream image(filename);
    if (!image) {
        std::cerr << "无法创建文件: " << filename << std::endl;
        return;
    }

    image << "P3\n" << width << " " << height << "\n255\n";

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int color = noiseToColor(heightmap[y][x]);
            image << color << " " << color << " " << color << " ";
        }
        image << "\n";
    }

    image.close();
    std::cout << "图像已保存: " << filename << std::endl;
}

// 生成彩色地形图（根据高度着色）
void saveColoredPPM(const std::string& filename,
    const std::vector<std::vector<double>>& heightmap,
    int width, int height) {

    std::ofstream image(filename);
    if (!image) {
        std::cerr << "无法创建文件: " << filename << std::endl;
        return;
    }

    image << "P3\n" << width << " " << height << "\n255\n";

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double value = clamp((heightmap[y][x] + 1.0) * 0.5, 0.0, 1.0);
            //value = std::pow(value, 2.5);
            int r, g, b;

            // 根据高度着色
            if (value < 0.1) {
                // 深水
                r = 0; g = 0; b = 100 + static_cast<int>(value * 155);
            }
            else if (value < 0.2) {
                // 浅水
                r = 30; g = 144; b = 255;
            }
            else if (value < 0.3) {
                // 沙滩
                r = 238; g = 214; b = 175;
            }
            else if (value < 0.6) {
                // 草地/森林
                int green = 100 + static_cast<int>(value * 155);
                r = 34; g = green; b = 34;
            }
            else if (value < 0.8) {
                // 岩石
                int gray = 100 + static_cast<int>(value * 100);
                r = gray; g = gray; b = gray;
            }
            else {
                // 雪
                r = 255; g = 255; b = 255;
            }

            image << r << " " << g << " " << b << " ";
        }
        image << "\n";
    }

    image.close();
    std::cout << "彩色地形图已保存: " << filename << std::endl;
}

/*int main() {
    const int width = 1024;
    const int height = 1024;

    // 创建分层噪声生成器
    LayeredNoise noiseGenerator(12345);

    std::vector<std::vector<double>> heightmap(height, std::vector<double>(width));

    // 生成地形
    std::cout << "生成山脉地形（1024x1024）..." << std::endl;

    double scale = 0.005;  // 调整这个值改变地形"缩放"

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // 使用5层噪声叠加
            double noiseValue = noiseGenerator.mountainNoise(x, y, scale);

            // 可选：添加其他效果
            // noiseValue = noiseGenerator.terrainNoise(x, y, scale);
            // noiseValue = noiseGenerator.islandNoise(x, y, scale);

            heightmap[y][x] = noiseValue;
        }

        // 显示进度
        if (y % (height / 10) == 0) {
            std::cout << "进度: " << (y * 100 / height) << "%" << std::endl;
        }
    }

    // 保存灰度图像
    savePPM("mountain_terrain.ppm", heightmap, width, height);

    // 保存彩色地形图
    saveColoredPPM("colored_terrain.ppm", heightmap, width, height);

    // 生成一个较小尺寸的测试图像
    std::cout << "\n生成测试图像（256x256）..." << std::endl;
    const int testWidth = 256;
    const int testHeight = 256;

    std::vector<std::vector<double>> testHeightmap(testHeight, std::vector<double>(testWidth));

    for (int y = 0; y < testHeight; y++) {
        for (int x = 0; x < testWidth; x++) {
            // 尝试不同的噪声函数
            double value;

            if (x < testWidth / 2) {
                // 左侧：基础5层噪声
                value = noiseGenerator.layeredNoise(x, y, 0.02, 0.5, 5);
            }
            else {
                // 右侧：地形噪声（带山脊效果）
                value = noiseGenerator.terrainNoise(x, y, 0.02);
            }

            testHeightmap[y][x] = value;
        }
    }

    savePPM("test_comparison.ppm", testHeightmap, testWidth, testHeight);

    // 显示一些统计信息
    double minVal = 1.0, maxVal = -1.0;
    double sum = 0.0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double val = heightmap[y][x];
            if (val < minVal) minVal = val;
            if (val > maxVal) maxVal = val;
            sum += val;
        }
    }

    std::cout << "\n地形统计信息:" << std::endl;
    std::cout << "最小值: " << minVal << std::endl;
    std::cout << "最大值: " << maxVal << std::endl;
    std::cout << "平均值: " << (sum / (width * height)) << std::endl;
    std::cout << "动态范围: " << (maxVal - minVal) << std::endl;

    return 0;
}
*/
