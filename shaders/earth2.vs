#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 anormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 atangent;

out vec2 TexCoord;
out vec3 TangentViewPos;
out vec3 TangentFragPos;
out vec3 TangentLightDir;
out vec3 TangentPointPos[4];

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPos;
uniform vec3 lightDir;  // 设置一个平行光源
uniform vec3 pointPose[4];   // 设定一组点光源
uniform int pointNum;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);

	TexCoord = vec2(aTexCoord.x, aTexCoord.y);

    mat3 normalMat = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMat * atangent);
    vec3 N = normalize(normalMat * anormal);
    vec3 B = normalize(cross(N, T));    // 通过切线和法向量计算副切线

    mat3 TBN = transpose(mat3(T, B, N));

    TangentViewPos = TBN * viewPos;
    TangentFragPos = TBN * vec3(model * vec4(aPos, 1.0));
  
    TangentLightDir = TBN * lightDir;

    for(int i=0; i<pointNum; i++)
        TangentPointPos[i] = TBN * pointPose[i];

	
}