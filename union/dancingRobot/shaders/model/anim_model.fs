#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 normal;
in vec3 position;
in vec3 tp;

struct DirLight{
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};


uniform DirLight dirLight;
uniform sampler2D texture_diffuse1;
uniform vec3 eyePos;

void main()
{   
    vec3 N=normalize(normal);
    vec3 L=normalize(-dirLight.direction);
    float NdotL=max(dot(N,L),0);
    vec3 V=normalize((eyePos-position));
    float NdotV=max(dot(N,V),0);

    // Diffuse shading
    float diff =NdotL;
    // Specular shading
    vec3 halfwayDir = normalize(-dirLight.direction + V);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 1);
    // Combine results
    vec3 ambient = dirLight.ambient * vec3(texture(texture_diffuse1, TexCoords));
    vec3 diffuse = dirLight.diffuse * diff *vec3(texture(texture_diffuse1, TexCoords));
    vec3 specular = dirLight.specular * spec *vec3(texture(texture_diffuse1, TexCoords));
    
   vec3 color=ambient+diffuse;
    if(NdotL<0.01)
        color=vec3(0.4,0.4,0.6)*color;
    else if(NdotL<0.1)
        color=vec3(0.5,0.5,0.7)*color;
    else if(NdotL<0.3)
        color=vec3(0.65,0.65,0.75)*color;
    else if(NdotL<0.6)
        color=vec3(0.85,0.85,0.95)*color;
    else if(NdotL<=0.99)
        color=color;
    else if(NdotL<0.995)
        color=vec3(0.89,0.85,0.85);
    else
        color=vec3(1,1,1);
    //ÂÖÀªÏß ¿É¿¼ÂÇÉ¾È¥
    if(NdotV<0.2)
        color=vec3(0.1,0.05,0.05);
  // else   //²âÊÔÂÖÀªÏß
      // color=vec3(1,1,1);

    FragColor = vec4(color,1.0);

}