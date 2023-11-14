#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragPos =  vec3(model * vec4(aPos, 1.0));

	TexCoord = vec2(aTexCoord.x, -aTexCoord.y);
	//TexCoord = vec2(aTexCoord.x * model[0][0], -aTexCoord.y * model[1][1]);
}