#version 330 core

out vec4 color;
in vec2 TexCoordsOut;
uniform sampler2D shadowMap;//“ı”∞Ã˘Õº


void main()
{
    float depth = texture(shadowMap,TexCoordsOut).r;
    //depth = 1.0 - (1.0 - depth) * 25.0;
    color = vec4(vec3(depth), 1.0f);
}