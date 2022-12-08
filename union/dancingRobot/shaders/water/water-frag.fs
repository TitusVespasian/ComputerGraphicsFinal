#version 410 core
out vec4 fragColor;
in vec2 texCoords;
in vec3 fragPos;
const int SHINENESS=512;
const vec3 WATER_COLOR=vec3(0.04,0.45,0.92);
struct Light{
	vec3 direction;
	vec3 diffuse;
	vec3 ambient;
	vec3 specular;
};
uniform sampler2D normalMap1;
uniform sampler2D normalMap2;
uniform sampler2D heightMap1;
uniform sampler2D heightMap2;
uniform sampler2D wavesNormalMap;
uniform sampler2D water;
uniform Light light;
uniform float interpolateFactor;
uniform float wavesOffset;
uniform vec3 viewPos;
vec3 calcLight(Light light,vec3 material,vec3 viewDir,vec3 normal);
vec3 calcNormal();
void main(){
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 normal = calcNormal();
	vec3 material =texture(water,vec2(texCoords.x,texCoords.y+wavesOffset)).rgb;
	fragColor = vec4(calcLight(light,material,viewDir,normal),1.0);
}
vec3 calcNormal(){
	vec3 normal1 = texture(normalMap1,vec2(texCoords.x,texCoords.y+wavesOffset)).rgb;
	vec3 normal2 = texture(normalMap2,vec2(texCoords.x,texCoords.y+wavesOffset)).rgb;
	vec3 waves = texture(wavesNormalMap,vec2(texCoords.x,texCoords.y+wavesOffset/5)).rgb;
	vec3 normal = mix(normal1,normal2,interpolateFactor).rgb;
	normal = mix(normal,waves,0.3).rgb;
	normal =normal.rbg*2-1.0;
	return normalize(normal);
}
vec3 calcLight(Light light, vec3 material, vec3 viewDir, vec3 normal){
	vec3 fragToLightDir = normalize(-light.direction);
	float diff =max(dot(fragToLightDir, normal),0.0);
	vec3 halfwayDir = normalize(fragToLightDir + viewDir);
	float specAngle = max(dot(halfwayDir,normal),0.0);
	float spec = pow(specAngle,SHINENESS);
	vec3 ambient = light.ambient * material;
	vec3 diffuse = light.diffuse * material *diff;
	vec3 specular = light.specular * spec;
	return ambient + diffuse + specular;
}