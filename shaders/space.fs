#version 330 core

// 定义一个平行光源
struct Light {
    //vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

// texture samplers
uniform sampler2D texture_diffuse;
uniform sampler2D texture_normal;
uniform sampler2D texture_specular;
uniform Light light;
uniform vec3 viewPos;



void main()
{
	
    // 获取法向
    // vec3 norm = texture(texture_normal, TexCoord).rgb * 2.0 - 1.0;

    // norm = normalize(norm);
    vec3 norm = normalize(Normal);

    // Ambient
    vec3 ambient = light.ambient * vec3(texture(texture_diffuse, TexCoord));


    // Diffuse
    vec3 lightDir = normalize(-light.direction);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse, TexCoord));

    // Specular
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm); 
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
	vec3 specular = light.specular * spec * vec3(texture(texture_specular, TexCoord));


	FragColor = vec4(ambient + diffuse + specular, 1.0f);
    //FragColor = vec4(ambient, 1.0f);
}
