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

out vec2 TexCoord;
out float face;

void main()
{
    TexCoord = aTexCoord;
    face = aFace;
    
    //实例化

    if (false) {
        gl_Position = projection * view * instanceMatrix * vec4(aPos+vec3(0.0,0.001,0.0), 1.0);
    } else {
        gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0);
    }
}  