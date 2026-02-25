#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in float aFace;
layout (location = 3) in int blockTextureID;//方块材质ID

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};
uniform mat4 model;//glm::mat4(1.0)

out vec2 TexCoord;
flat out int face;
flat out int BlockTextureID;

void main()
{
    TexCoord = aTexCoord;
    face = int(round(aFace));
    BlockTextureID = blockTextureID;

    gl_Position = projection * view * vec4(aPos, 1.0);
}  