#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
 
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;//光空间变换矩阵
 
uniform Mat{
	vec4 aAmbient;
	vec4 aDiffuse;
	vec4 aSpecular;
};
out vec3 FragPos;//普通空间的坐标
out vec3 Normal;
out vec2 TexCoords;
out vec4 FragPosLightSpace;//光空间的坐标
 
out vec4 Ambient;
out vec4 Diffuse;
out vec4 Specular;
 
void main()
{
    FragPos = vec3( model * vec4(aPos, 1.0));
	Normal = mat3(transpose(inverse(model))) * aNormal;
	Ambient = aAmbient;
	Diffuse = aDiffuse;
	Specular = aSpecular;
 
    TexCoords = aTexCoords;    
    gl_Position = projection * view * model * vec4(aPos, 1.0);
	FragPosLightSpace=lightSpaceMatrix*vec4(FragPos,1.0);
}