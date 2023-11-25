#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 texCoords;
    vec3 FragPosition;
} gs_in[];

out vec2 TexCoord; 
out vec3 FragPos;

uniform float time;

vec4 explode(vec4 position, vec3 normal)
{
    float magnitude = 3.5;
    vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude; 
    return position - vec4(direction, 0.0);
}

vec3 GetNormal()
{
    vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
    return normalize(cross(a, b));
}

void main() {    
    vec3 normal = GetNormal();

    gl_Position = explode(gl_in[0].gl_Position, normal);
    TexCoord = gs_in[0].texCoords;
    FragPos = gs_in[0].FragPosition;
    EmitVertex();
    gl_Position = explode(gl_in[1].gl_Position, normal);
    TexCoord = gs_in[1].texCoords;
    FragPos = gs_in[1].FragPosition;
    EmitVertex();
    gl_Position = explode(gl_in[2].gl_Position, normal);
    TexCoord = gs_in[2].texCoords;
    FragPos = gs_in[2].FragPosition;
    EmitVertex();
    EndPrimitive();
}