#version 330 core


out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;


// 点光源设定的参数
struct PointLight {
    // 颜色参数
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    // 点光源衰减因子参数
    float constant;
    float linear;
    float quadratic;
};


uniform PointLight LightInfo;
// texture samplers
uniform sampler2D texture_diffuse;
uniform sampler2D texture_normal;
uniform sampler2D texture_specular;
uniform int lightNum;    // 点光源的个数
uniform vec3 PointPos[4];
uniform vec3 viewPos;



void main()
{
	
    vec4 NormMap = texture(texture_normal, TexCoord).rgba;
    vec4 DiffMap = texture(texture_diffuse, TexCoord).rgba;
    vec4 SpecMap = texture(texture_specular, TexCoord).rgba;

    // 获取 normal map 中的法向量的值
    vec3 norm = normalize(Normal);

    // 光滑度
    float Shininess = 16.0 * SpecMap.a;
    // Speculat 强度
    float SpecularStrength = SpecMap.g * 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 Color = vec3(0., 0., 0.);

    for(int i = 0; i < lightNum; i++){
        // 计算衰减因子
        float distance = length(PointPos[i] - FragPos);
        float attenuation = 1.0f / (LightInfo.constant + LightInfo.linear*distance + LightInfo.quadratic*(distance*distance) );

        vec3 lightDir = normalize(PointPos[i] - FragPos);
        vec3 halfwayDir = normalize(viewDir + lightDir);
        //漫反射着色
        float diff = max(dot(norm, lightDir), 0.0);
        //镜面光着色
        vec3 reflectDir = reflect(-lightDir, norm);
       
        float spec = pow(max(dot(norm, halfwayDir), 0.0), Shininess);
        // 合并结果
        vec3 ambient  = LightInfo.ambient  * DiffMap.rgb;
        vec3 diffuse  = LightInfo.diffuse  * diff * DiffMap.rgb;
        vec3 specular = LightInfo.specular * spec * vec3(SpecularStrength);

        // 计算衰减后的光照结果
       ambient *= attenuation;
       diffuse *= attenuation;
       specular *= attenuation;

        Color += max((1.0 - NormMap.b) * (ambient + diffuse + specular), 0.0) + (NormMap.b) * DiffMap.rgb;
    }

	FragColor = vec4( Color, DiffMap.a);
   
}
