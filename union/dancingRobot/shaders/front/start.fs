#version 330 core

in vec2 fragCoord;

out vec4 fragColor;

uniform float Time;
uniform float ETime;
uniform bool End;


bool letterN(ivec2 n)
{
	return (n.x==0 || n.x==3) && n.y<5 && n.y>-1 || n.x==1 && n.y<4 && n.y>1 || n.x==2 && n.y<3 && n.y>0;
}

bool letterU(ivec2 n)
{
	return (n.x==0 || n.x==3) && n.y<5 && n.y>-1 || n.x==1 && (n.y==0) || n.x==2 && (n.y==0);
}
	  
bool letterT(ivec2 n)
{
	return (n.x==0 || n.x==1 || n.x==3) && (n.y==4) || n.x==2 && (n.y<5 && n.y>-1);
}	 

bool letterJ(ivec2 n)
{
	return (n.x==0 || n.x==1) && (n.y==4 || n.y==0) || (n.x==3) && (n.y==4) || n.x==2 && (n.y<5 && n.y>-1);
}

bool letterS(ivec2 n)
{
    return (n.x==0) && (n.y==0 || n.y==2 || n.y==3 || n.y==4) || (n.x==3) && (n.y==0 || n.y==2 || n.y==4 || n.y==1) || (n.x==1 || n.x==2) && (n.y==0 || n.y==2 || n.y==4);
}

bool letterD(ivec2 n)
{
    return (n.x==0) && n.y<5 && n.y>-1 || (n.x==1) && (n.y==0 || n.y==4) || (n.x==2) && (n.y==1 || n.y==4) || (n.x==3) && (n.y==2 || n.y==3 || n.y==4) ;
}

bool letterE(ivec2 n)
{
	return (n.x==0) && n.y<5 && n.y>-1 || n.x==1 && (n.y==0 || n.y == 2 || n.y==4) || n.x==2 && (n.y==0 || n.y == 2 || n.y==4) || n.x==3 && (n.y==0 || n.y==4);
}

bool letterC(ivec2 n)
{
	return (n.x==0) && n.y<5 && n.y>-1 || n.x==1 && (n.y==0 || n.y==4) || n.x==2 && (n.y==0 || n.y==4) || n.x==3 && (n.y==0 || n.y==4);
}

bool letterA(ivec2 n)
{
	return (n.x==0 || n.x==3) && n.y<5 && n.y>-1 || n.x==1 && (n.y==4 || n.y == 2) || n.x==2 && (n.y==4 || n.y == 2);
}

void main()
{
    vec2 p = fragCoord;
    float d = 1.0 - (length(vec2(p.x, p.y)) - cos(ETime/1.9) * 0.5) * 2.0;
    vec3 grayColor = vec3(d);

    float t = Time;
    p *= sin(5.0 * t) * 20.0 + 40.0;
    vec2 n = floor(p);

    float dist = 0.0;
    dist = 1.0 / length(p - (n + 0.5));

    n += 5.0;
    n.x += 5.00;
    ivec2 ni = ivec2(n.x, n.y - 6.0);

    float scale = 0.35;
    if (sin(t * 4.0) > 0.0)
    {
        dist *= scale;
        scale = 1.0 / scale;
    }


    if (letterJ(ni) || letterU(ivec2(ni.x - 5, ni.y)) || letterS(ivec2(ni.x - 10, ni.y)) || letterT(ivec2(ni.x - 15, ni.y)))
        dist *= scale;

    ni = ivec2(n.x + 2.0, n.y);

    if (letterD(ni) || letterA(ivec2(ni.x - 5, ni.y)) || letterN(ivec2(ni.x - 10, ni.y)) || letterC(ivec2(ni.x - 15, ni.y)) || letterE(ivec2(ni.x - 20, ni.y)))
        dist *= scale;

    vec3 c1 = vec3(1.0, 0.412, 0.706);
    vec3 c2 = vec3(0.698, 0.2275, 0.9333);

    vec3 c=c1;

	if(sin(t*2.0)>0.0)
		c = c2;

    if(End)
    {
        fragColor = vec4(grayColor, 1.0);
	    fragColor *= vec4(c*dist,1.0);
    }
    else
        fragColor = vec4(c*dist,1.0);
    
}