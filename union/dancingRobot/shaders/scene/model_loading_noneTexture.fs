#version 330 core
out vec4 FragColor;
 
in vec2 TexCoords;
 
uniform sampler2D texture_diffuse1;
uniform sampler2D shadowMap;//阴影贴图
 
struct PointLight{
    vec3 position;  
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	
    float constant;
    float linear;
    float quadratic;
};

struct DirLight{
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;  
in vec3 Normal;
in vec4 FragPosLightSpace;
//从Mtl中读取的数据
//Material
in vec4 Ambient;
in vec4 Diffuse;
in vec4 Specular;
//点光源数量
#define NR_POINT_LIGHTS 1

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform bool is_pointlight;//是否要设置点光源
 
uniform float shininess;

// Function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir,float shadow);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir,float shadow);
float ShadowCalculation(vec4 fragPosLightSpace, float bias);
 
void main()
{    
    // Properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // Phase 1: Directional lighting
   // float bias = max(0.0013 * (1.0 - dot(norm, normalize(-dirLight.direction))), 0.001);
   float bias = max(0.03 * (1.0 - dot(norm, normalize(-dirLight.direction))), 0.001);
    float shadow = ShadowCalculation(FragPosLightSpace,bias); 

    vec3 result = CalcDirLight(dirLight, norm, viewDir,shadow);
    // Phase 2: Point lights
    for(int i = 0; is_pointlight&&(i < NR_POINT_LIGHTS); i++)
       result += CalcPointLight(pointLights[i], norm, FragPos, viewDir,shadow);    

    FragColor = vec4(result ,1.0);
}

float ShadowCalculation(vec4 fragPosLightSpace, float bias)
{
    // 执行透视除法
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 变换到[0,1]的范围
    projCoords = projCoords * 0.5 + 0.5;
   if(projCoords.z > 1.0)
        return 0.0;
    // 取得最近点的深度(使用[0,1]范围下的fragPosLight当坐标)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // 取得当前片元在光源视角下的深度
    float currentDepth = projCoords.z;
    // 检查当前片元是否在阴影中

    //进行光滑
    float shadow = 0.0;
    //这个textureSize返回一个给定采样器纹理的0级mipmap的vec2类型的宽和高。用1除以它返回一个单独纹理像素的大小，
       //我们用以对纹理坐标进行偏移，确保每个新样本，来自不同的深度值。
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
    	for(int y = -1; y <= 1; ++y)
    	{
           float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
           shadow += currentDepth- bias > pcfDepth ? 1.0 : 0.0;    //  
    	}    
    }
    shadow /= 9.0;


    return shadow;
}
// Calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir,float shadow)
{
    vec3 lightDir = normalize(-light.direction);
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 16.0);
    // Combine results
    vec3 ambient = light.ambient * Diffuse.rgb;
    vec3 diffuse = light.diffuse * diff * Diffuse.rgb;
    vec3 specular = light.specular * spec * Specular.rgb;

    
    if(shadow<0)
        return (ambient + diffuse + specular);
    else
        return (ambient + (1.0 - shadow) * (diffuse + specular));    
    //try to render cartoon style
    //if(diff<0.5)
    //    return ambient;
    //else if(diff<0.85)
    //    return diffuse;
    //else 
    //    return specular;
}

// Calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir,float shadow)
{
    vec3 lightDir = normalize(light.position - FragPos);
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 16.0);
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    

   // Combine results
    vec3 ambient = light.ambient * Diffuse.rgb;
    vec3 diffuse = light.diffuse * diff * Diffuse.rgb;
    vec3 specular = light.specular * spec * Specular.rgb;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    //try to render cartoon style
    //if(diff<0.5)
    //    return ambient;
    //else if(diff<0.85)
    //    return diffuse;
    //else 
    //    return specular;
    if(shadow<0.2)
        return (ambient + diffuse + specular);
    else
        return vec3(0);    

}