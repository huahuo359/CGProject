#version 330

in vec2 st;

uniform sampler2D texMap;

out vec4 out_colour;

void main(void){
    out_colour = texture(texMap, st);
//    out_colour = vec4(0.5,0.0f,0.0f,1.0f);
}