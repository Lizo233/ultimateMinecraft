#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in float aFace;
layout (location = 3) in int blockTextureID;   // 方块材质ID

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};
uniform mat4 model;   // 通常为单位矩阵

out vec2 TexCoord;
flat out int face;
flat out int BlockTextureID;
out float FragDistance;   // 新增：视空间距离

void main()
{
    TexCoord = aTexCoord;
    face = int(round(aFace));
    BlockTextureID = blockTextureID;

    // 计算视空间坐标
    vec4 viewPos = view * model * vec4(aPos, 1.0);
    FragDistance = length(viewPos.xyz);   // 到摄像机的距离

    gl_Position = projection * viewPos;
}