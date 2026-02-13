#version 330 core
uniform sampler2D texture0;
uniform sampler2DArray uTextureArray;

in vec2 TexCoord;
in float face;

out vec4 FragColor;

//FragColor = texture(texture0, TexCoord*0.5+vec2(0,0.5));//侧面
//FragColor = texture(texture0, TexCoord*0.5+vec2(0.5,0.5));//底部
//FragColor = texture(texture0, TexCoord*0.5+vec2(0,0));//顶部



void main()
{
    if (face == 0.0) {
        FragColor = texture(texture0, TexCoord*0.5+vec2(0,0.5));//侧面
    } else if (face == -1.0) {
        FragColor = texture(texture0, TexCoord*0.5+vec2(0.5,0.5));//底部
    } else if (face == 1.0) {
        FragColor = vec4(vec3(texture(texture0, TexCoord*0.5+vec2(0,0)).r*0.63,texture(texture0, TexCoord*0.5+vec2(0,0)).r*1.0,texture(texture0, TexCoord*0.5+vec2(0,0)).r*0.54)*0.8,1.0);//顶部
        //TMD这简直完全一样，哈哈哈哈，我是天才！
    }
    //MSAA测试（纯色）
    //FragColor = vec4(0.0,0.8,0.0,1.0);

    //材质数组
    //FragColor = texture(uTextureArray, vec3(TexCoord, 0));
}