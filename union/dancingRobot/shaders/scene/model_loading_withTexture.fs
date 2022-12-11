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
float Calculate_Avg_Dblockreceiver(vec2 projCoords_xy , int AvgTextureSize);
float PCSS(vec4 fragPosLightSpace,float bias);
 
void main()
{    
    // Properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // Phase 1: Directional lighting
    float bias = max(0.003 * (1.0 - dot(norm, normalize(-dirLight.direction))), 0.001);
    float shadow = PCSS(FragPosLightSpace,bias); 

    vec3 result = CalcDirLight(dirLight, norm, viewDir,shadow);
    // Phase 2: Point lights
    for(int i = 0; is_pointlight&&(i < NR_POINT_LIGHTS); i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir,shadow);    
    FragColor = vec4(result ,1.0);
}

float Calculate_Avg_Dblockreceiver(vec2 projCoords_xy , int AvgTextureSize)
{
    vec2 texelSize =1.0/ textureSize(shadowMap, 0);
    float result=0.0f;
    for(int i=-AvgTextureSize;i<=AvgTextureSize;++i)
    {
        for(int j=-AvgTextureSize;j<=AvgTextureSize;j++)
        {
            result += texture(shadowMap, projCoords_xy+vec2(i,j)*texelSize).r; 
        }
    }
    return result/(AvgTextureSize*AvgTextureSize*2*2);
}

//PCSS
float PCSS(vec4 fragPosLightSpace,float bias)
{
    float lightRadius=10.0;
    float shadow=0;
    vec3 lightDir =normalize(dirLight.direction);
    vec2 texelSize =1.0/ textureSize(shadowMap, 0);
    // 执行透视除法
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 转换到 [0,1]
    projCoords = projCoords *0.5+0.5;
    if(projCoords.z > 1.0)
        return 0.0;
    // 采样ShadowMap中的深度
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // 获取当前着色点的深度
    float currentDepth = projCoords.z;
  
    //计算着色点与平均遮挡物的距离 dr
    float D_light_block =Calculate_Avg_Dblockreceiver(projCoords.xy,7);
    float D_block_receiver= (currentDepth- D_light_block );
    // 检查当前点是否在阴影中
    if( D_light_block<0.01f)
        return 0.0f;
    //利用平均遮挡物距离dr计算PCF用到的采样范围 Wsample
    float fliterArea=D_block_receiver/(D_light_block*fragPosLightSpace.w) *lightRadius;
    int fliterSingleX=int(fliterArea);
    int count=0;
    fliterSingleX = fliterSingleX >10?10: fliterSingleX;
    fliterSingleX = fliterSingleX <1?2: fliterSingleX;
    //计算PCF
    for(int i=-fliterSingleX;i<=fliterSingleX;++i)
    {
        count++;
        for(int j=-fliterSingleX;j<=fliterSingleX;j++)
        {
            //  采样周围点在ShadowMap中的深度
            float closestDepth = texture(shadowMap, projCoords.xy+vec2(i,j)*texelSize).r; 
            shadow += currentDepth-bias > closestDepth  ?1.0:0.0;
        }
    }
    count = count >0? count :1;
    shadow = shadow/float(count*count);
    
    
    return shadow ;
}
// Calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir,float shadow)
{
    vec3 lightDir = normalize(-light.direction);
    // Diffuse shading
    float diff =max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 16.0);
    // Combine results
    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff *vec3(texture(texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec *vec3(texture(texture_diffuse1, TexCoords));

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
    vec3 ambient = light.ambient *vec3(texture(texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff *vec3(texture(texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec *vec3(texture(texture_diffuse1, TexCoords));
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