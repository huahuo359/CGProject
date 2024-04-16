#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;


out VS_OUT {
    vec2 texCoords;
    vec3 FragPosition;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
    vs_out.texCoords = aTexCoord;
    vs_out.FragPosition = vec3(model * vec4(aPos, 1.0));
    
}