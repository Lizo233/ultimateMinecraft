#pragma once
#include <main.h>


//着色器类
class ShaderProgram {
public:
	//保存着色器程序的ID
	uint32_t shaderProgramID;

	ShaderProgram(const std::string vertexShaderPath, const std::string fragmentShaderPath) {

		std::string vertShaderCode, fragShaderCode;
		std::fstream vertShaderFile, fragShaderFile;

		//确保在文件不存在的情况下能抛出异常
		vertShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fragShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try {
			vertShaderFile.open("assets/cube.vert");
			fragShaderFile.open("assets/cube.frag");

			std::stringstream vertStream, fragStream;

			vertStream << vertShaderFile.rdbuf();
			fragStream << fragShaderFile.rdbuf();

			vertShaderCode = vertStream.str();
			fragShaderCode = fragStream.str();

		}
		catch (std::fstream::failure& e) {
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
			logError("无法打开着色器文件 " + (std::string)e.what());
		}

		//加载完成之后开始编译
		uint32_t vertexShader, fragmentShader;
		const char* shaderCode = vertShaderCode.c_str();

		//顶点着色器编译
		vertexShader = glCreateShader(GL_VERTEX_SHADER);//创建顶点着色器
		glShaderSource(vertexShader, 1, &shaderCode, nullptr);//加载着色器源码
		glCompileShader(vertexShader);//编译
		checkCompileErrors(vertexShader, "VERTEX");

		//片元着色器
		shaderCode = fragShaderCode.c_str();
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);//创建片元着色器
		glShaderSource(fragmentShader, 1, &shaderCode, nullptr);//加载着色器源码
		glCompileShader(fragmentShader);
		checkCompileErrors(fragmentShader, "FRAGMENT");

		//附加着色器到程序
		shaderProgramID = glCreateProgram();//创建着色器程序
		glAttachShader(shaderProgramID, vertexShader);
		glAttachShader(shaderProgramID, fragmentShader);
		glLinkProgram(shaderProgramID);//将它们链接
		checkCompileErrors(shaderProgramID, "PROGRAM");


		//着色器连接到着色器程序之后我们就不需要着色器了
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

	}
	
	//激活该shader
	void active() const {
		glUseProgram(shaderProgramID);
	}

	//各种uniform变量的设置

	void setUniMat4(const std::string& name, const glm::mat4& matrix) const {
		glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, name.c_str()), 1, GL_FALSE, &matrix[0][0]);
	}

	void setUniInt(const std::string& name, const int value) const {
		glUniform1i(glGetUniformLocation(shaderProgramID, name.c_str()), value);
	}


private:
	void checkCompileErrors(uint32_t shader, std::string type) {
		int success;
		char infoLog[1024];
		if (type != "PROGRAM") {
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
				std::cerr << "错误::着色器编译错误，着色器类型：" + type + '\n' + infoLog << std::endl;
				logError("错误::着色器编译错误，着色器类型：" + type + '\n' + infoLog);

			}
		}
		else {
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
				std::cerr << "错误::着色器连接错误，着色器类型：" + type + '\n' + infoLog << std::endl;
				logError("错误::着色器连接错误，着色器类型：" + type + '\n' + infoLog);
			}
		}
	}
};

//着色器初始化
ShaderProgram* cubeShader = nullptr;
void shaderInit() {
	cubeShader = new ShaderProgram("assets/cube.vert", "assets/cube.frag");



	logInfo("着色器初始化完成");
}

//生成ubo
uint32_t uboMatrices;//设置为全局变量方便读取
void uboInit() {

	//把这个shaderProgram的对应的UniformBlock绑定到BindingPoint0
	glUniformBlockBinding(cubeShader->shaderProgramID, glGetUniformBlockIndex(cubeShader->shaderProgramID, "Matrices"), 0);
	glGenBuffers(1, &uboMatrices);//生成ubo
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);//绑定ubo
	//将ubo的大小分分配为2个mat4矩阵的大小
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);//解绑ubo

	//绑定ubo的显存起点为BindingPoint0的地址
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

	//投影矩阵只需要计算一次（如果不改变FOV的话）
	glm::mat4 projection = glm::perspective(45.0, (double)DEFAULT_WIDTH / DEFAULT_HEIGHT, 0.1, 1000.0);
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);//绑定ubo
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));//传输矩阵到显存
	glBindBuffer(GL_UNIFORM_BUFFER, 0);//解绑ubo
}