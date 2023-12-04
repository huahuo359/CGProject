#version 330 core


out vec4 FragColor;

in vec2 TexCoord;
in vec3 TangentViewPos;
in vec3 TangentFragPos;
in vec3 TangentLightDir;
in vec3 TangentPointPos[4];

// 点光源设定的参数
struct PointLight {
    // 辐射参数
    // float constant;
    // float linear;
    // float quadratic;
    // 颜色参数
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};


uniform PointLight LightInfo;
// texture samplers
uniform sampler2D texture_diffuse;
uniform sampler2D texture_normal;
uniform sampler2D texture_specular;
uniform int pointNum;



void main()
{
	
    vec4 NormMap = texture(texture_normal, TexCoord).rgba;
    vec4 DiffMap = texture(texture_diffuse, TexCoord).rgba;
    vec4 SpecMap = texture(texture_specular, TexCoord).rgba;

    // 获取 normal map 中的法向量的值
    // vec3 norm = vec3(NormMap.ra*2.0 - 1.0, 0.0);
    // norm.z = sqrt(max(0.0, 1.0 - norm.x * norm.x - norm.y * norm.y));

    vec3 norm = vec3(NormMap.ra * 2.0 - 1.0, 0.0);
    norm.z = sqrt(1.0 - norm.x * norm.x - norm.y * norm.y);


    // 光滑度
    float Shininess = 16.0 * SpecMap.a;
    // Speculat 强度
    float SpecularStrength = SpecMap.g * 0.5;

    // 计算 rgb
    vec3 viewDir = normalize(TangentViewPos - TangentFragPos);
    
    vec3 ambient = LightInfo.ambient * DiffMap.rgb;

    

    float diff = max(dot(norm, normalize(TangentLightDir)), 0.0);
    vec3 diffuse = LightInfo.diffuse * diff * DiffMap.rgb;

    vec3 halfWayDir = normalize(normalize(TangentLightDir) + viewDir);
    float spec = pow(max(dot(norm, halfWayDir), 0.0), Shininess);
    vec3 specular = LightInfo.specular * spec * vec3(SpecularStrength);
   

    vec3 Color = vec3(0., 0., 0.);
    Color += max((1.0 - NormMap.b) * (ambient + diffuse + specular), 0.0) + (NormMap.b) * DiffMap.rgb;
    //Color += max((1.0 - NormMap.b) * (specular2 + diffuse2), 0.0) + (NormMap.b) * DiffMap.rgb;
    

    for(int i = 0; i < pointNum; i++){
        vec3 lightDir = normalize(TangentPointPos[i] - TangentFragPos);
        //漫反射着色
        float diff = max(dot(norm, lightDir), 0.0);
        //镜面光着色
        vec3 reflectDir = reflect(-lightDir, norm);
        vec3 halfWayDir = normalize(normalize(lightDir) + viewDir);
        float spec = pow(max(dot(norm, halfWayDir), 0.0), Shininess);
        // 合并结果
        vec3 ambient  = LightInfo.ambient  * DiffMap.rgb;
        vec3 diffuse  = LightInfo.diffuse  * diff * DiffMap.rgb;
        vec3 specular = LightInfo.specular * spec * vec3(SpecularStrength);
      
        Color += max((1.0 - NormMap.b) * (ambient + diffuse + specular), 0.0) + (NormMap.b) * DiffMap.rgb;
        //Color += (ambient + diffuse + specular);
    }

	FragColor = vec4( Color, DiffMap.a);
   
}
