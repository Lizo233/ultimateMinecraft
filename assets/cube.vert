#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in float aFace;

layout (location = 3) in vec3 instanceVector;//实例化数组

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};
uniform mat4 model;

out vec2 TexCoord;
flat out int face;

//矩阵位移函数
mat4 translate(mat4 m,vec3 t) {
    m[3] = m[0] * t.x + m[1] * t.y + m[2] * t.z + m[3];
    return m;
}

void main()
{
    TexCoord = aTexCoord;
    face = int(round(aFace));

    //实例化

    // 构造位移矩阵
    mat4 trans = mat4(1.0, 0.0, 0.0, 0.0,
                      0.0, 1.0, 0.0, 0.0,
                      0.0, 0.0, 1.0, 0.0,
                      instanceVector, 1.0);

    gl_Position = projection * view * trans * vec4(aPos, 1.0);
}  