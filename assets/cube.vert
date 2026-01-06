#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in int aFace;
layout (location = 3) in mat4 instanceMatrix;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};
uniform mat4 model;
uniform mat4 offsets[100];

out vec2 TexCoord;
out float face;

void main()
{
    TexCoord = aTexCoord;
    face = aFace;
    
    //สตภýปฏ
    mat4 offsetModel = offsets[gl_InstanceID] * model;

    gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0);
}  