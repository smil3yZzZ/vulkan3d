#version 450

layout(set = 0, binding = 0) uniform UVReflectionBufferUbo {
	mat4 projection;
	mat4 view;
	vec2 invResolution;
} ubo;
layout (binding = 1) uniform sampler2DArray samplerMappingsMap;

layout(push_constant) uniform Push {
	mat4 modelMatrix;
	float reflection;
} push;

layout (location = 0) out vec4 outUVReflection;

float maxDistance = 15;
float resolution  = 0.3;
int   steps       = 10;
float thickness   = 0.5;

void main() {
	// Read previous pass shadow depth & G-Buffer values from previous sub pass
	vec2 clipUV = gl_FragCoord.xy * ubo.invResolution;
	vec2 clipXY = clipUV * 2.0 - 1.0;

	// Screen-space mappings start
	
	//Mappings variables
	vec4 positionFrom = texture(samplerMappingsMap, vec3(clipUV, 0));
	vec3 unitPositionFrom = normalize(positionFrom.xyz);
	vec3 reflectionNormal = normalize(texture(samplerMappingsMap, vec3(clipUV, 1)).xyz);
	vec3 pivot = normalize(reflect(unitPositionFrom, reflectionNormal));

	vec4 startView = vec4(positionFrom.xyz + (pivot * 0), 1.0);
	vec4 endView = vec4(positionFrom.xyz + (pivot * maxDistance), 1.0);

	vec4 startFrag = startView;
	// Project to screen space.
	startFrag = ubo.projection * startFrag;
	// Perform the perspective divide.
	startFrag.xyz /= startFrag.w;
	// Convert the screen-space XY coordinates to UV coordinates.
	startFrag.xy = startFrag.xy * 0.5 + 0.5;
	// Convert the UV coordinates to fragment/pixel coordnates.
	startFrag.xy /= ubo.invResolution;

	vec4 endFrag = endView;
    endFrag = ubo.projection * endFrag;
    endFrag.xyz /= endFrag.w;
    endFrag.xy = endFrag.xy * 0.5 + 0.5;
    endFrag.xy /= ubo.invResolution;
	
	// Screen-space mappings end

	outUVReflection = positionFrom;
}