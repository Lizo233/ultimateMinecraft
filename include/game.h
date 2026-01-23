#include <main.h>

class Chunk {
	//这应该够多了吧，加mod什么的话应该也可以...吧？
	//不行，感觉还是要调色板才行
	uint16_t blocks[16][256][16];//最多65536种方块，这里写的是区块中位于[x][y][z]的方块的方块ID（空气:0,石头:1 ...）

};

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
