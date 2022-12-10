#version 330 core

uniform sampler2D texture0;
//uniform vec2 iResolution;
uniform float iTime;

in vec2 fragCoord;

out vec4 FragColor;

float dfBox2(vec2 p, float r)
{
    return length(p) - r;
}

void main()
{
    //vec2 uv = fragCoord.xy / iResolution.xy;

    vec2 p = fragCoord - 0.5;

    float d = 1.0 - dfBox2(vec2(p.x, p.y), cos(iTime) * 0.4) * 2.0;
    vec3 grayColor = vec3(d);

    FragColor = vec4(grayColor, 1.0);
    FragColor *= texture(texture0, fragCoord);
}
