#version 330 core
out vec4 FragColor;
 
in vec2 TexCoords;
 
uniform sampler2D texture_diffuse1;
 
struct Light{
    vec3 position;  
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	
    float constant;
    float linear;
    float quadratic;
};
in vec3 FragPos;  
in vec3 Normal;
//从Mtl中读取的数据
//Material
in vec4 Ambient;
in vec4 Diffuse;
in vec4 Specular;
 
uniform vec3 viewPos;
uniform Light light;
 
uniform float shininess;
 
void main()
{    
    // ambient
    vec3 ambient = light.ambient * Diffuse.rgb;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse =light.diffuse * diff *Diffuse.rgb;  
    
    // attenuation
    float distance    = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)); 
    
	// specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = light.specular * spec *   Specular.rgb;  
	
    ambient  *= attenuation;
    diffuse   *= attenuation;
    specular *= attenuation;
	
    vec3 result = ambient + diffuse +specular;
    FragColor = vec4(result ,1.0);
	
    //FragColor = texture(texture_diffuse1, TexCoords);
}