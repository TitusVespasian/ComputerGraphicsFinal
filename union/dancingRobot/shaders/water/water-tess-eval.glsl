#version 410 core
layout (quads, equal_spacing, ccw) in;
out vec2 texCoords;
out vec3 fragPos;
uniform sampler2D heightMap1;
uniform sampler2D heightMap2;
uniform sampler2D wavesHeightMap;
uniform float depth;
uniform mat4 mvp;
uniform mat4 model;
uniform float wavesOffset;
uniform float interpolateFactor;
float calcHeight(vec2 tc);
void main(){
	vec2 tc=gl_in[0].gl_Position.xz;
	tc=vec2(tc.x+gl_TessCoord.x/64.0,tc.y+gl_TessCoord.y/64.0);
	vec4 tessellatedPoint = vec4(tc.x*2-1, calcHeight(tc), tc.y*2-1, 1.0);
	
	gl_Position = mvp * tessellatedPoint;
	fragPos = vec3(model * tessellatedPoint);
	texCoords = tc;
}
float calcHeight(vec2 tc){
	float height1 = texture(heightMap1, vec2(tc.x,tc.y+wavesOffset)).r;
	float height2 = texture(heightMap2, vec2(tc.x,tc.y+wavesOffset)).r;
	float wavesHeight = texture(wavesHeightMap, vec2(tc.x,tc.y + wavesOffset/5)).r;
	float height = mix(height1,height2, interpolateFactor);
	height = mix(height, wavesHeight,0.3);
	height=height*2-1;
	height *= depth;
	return height;
}