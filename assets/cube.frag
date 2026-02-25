#version 330 core
uniform sampler2D texture0;          // 可能未使用
uniform sampler2DArray uTextureArray;

in vec2 TexCoord;
flat in int face;
flat in int BlockTextureID;
in float FragDistance;                // 从顶点着色器传入的距离

out vec4 FragColor;

// 线性雾参数
uniform vec3 fogColor;                // 雾的颜色（应与 glClearColor 一致）
uniform float fogStart;                // 雾开始的距离
uniform float fogEnd;                  // 雾完全覆盖的距离

void main()
{
    // 原始纹理采样（根据面类型选择不同纹理层）
    if (face >= 0 && face <= 3) {       // 东南西北面 (0-3)
        FragColor = texture(uTextureArray, vec3(TexCoord, 1));
    } else if (face == 5) {             // 底面
        FragColor = texture(uTextureArray, vec3(TexCoord, 2));
    } else if (face == 4) {             // 顶面
        float color = texture(uTextureArray, vec3(TexCoord, 0)).r;
        FragColor = vec4(color*0.63, color*0.95, color*0.432, 1.0);
    }

    // ---------- 线性雾计算 ----------
    float dist = FragDistance;
    // 计算雾因子：在 start 之前为 1（无雾），在 end 之后为 0（完全雾），中间线性插值
    float fogFactor = (fogEnd - dist) / (fogEnd - fogStart);
    fogFactor = clamp(fogFactor, 0.0, 1.0);   // 限制在 [0,1]

    // 混合：最终颜色 = 雾色 * (1 - fogFactor) + 原色 * fogFactor
    FragColor.rgb = mix(fogColor, FragColor.rgb, fogFactor);
    // 保留原始 alpha（如果后续需要透明物体可调整）
}