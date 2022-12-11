#version 330 core

uniform sampler2D texture;
uniform float Time;

in vec2 fragCoord;

out vec4 FragColor;


void main()
{

    vec2 p = fragCoord - 0.5;

    float d = 1.0 - (length(vec2(p.x, p.y)) - cos(Time/1.9) * 0.5) * 2.0;
    vec3 grayColor = vec3(d);

    FragColor = vec4(grayColor, 1.0);
    FragColor *= texture(texture, fragCoord);
}
