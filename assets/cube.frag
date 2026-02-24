#version 330 core
uniform sampler2D texture0;
uniform sampler2DArray uTextureArray;

in vec2 TexCoord;
flat in int face;
flat in int BlockTextureID;

out vec4 FragColor;



void main()
{
//    if (face == 0.0) {
//        FragColor = texture(texture0, TexCoord*0.5+vec2(0,0.5));//侧面
//    } else if (face == -1.0) {
//        FragColor = texture(texture0, TexCoord*0.5+vec2(0.5,0.5));//底部
//    } else if (face == 1.0) {
//        FragColor = vec4(vec3(texture(texture0, TexCoord*0.5+vec2(0,0)).r*0.63,texture(texture0, TexCoord*0.5+vec2(0,0)).r*1.0,texture(texture0, TexCoord*0.5+vec2(0,0)).r*0.54)*0.8,1.0);//顶部
//        //TMD这简直完全一样，哈哈哈哈，我是天才！
//    }

    //材质数组
    if (face >= 0 && face <= 3) {//东南西北面 (0-3)
        FragColor = texture(uTextureArray, vec3(TexCoord, 1));
    } else if (face == 5) {//底面
        FragColor = texture(uTextureArray, vec3(TexCoord, 2));
    } else if (face == 4) {//顶面
        float color = texture(uTextureArray, vec3(TexCoord, 0)).r;
        FragColor = vec4(color*0.63,color*0.95,color*0.432,1.0);
    }
}