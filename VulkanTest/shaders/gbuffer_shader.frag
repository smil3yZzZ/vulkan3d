#version 450

layout (location = 0) in vec4 fragColor;
layout (location = 1) in vec4 fragNormalWorld;
layout (location = 2) in vec4 vecToLight;
layout (location = 3) in float vecToLightLength;
layout (location = 4) in vec3 lightPos;
layout (location = 5) in vec4 posWorld;

layout (location = 0) out vec4 outNormal;
layout (location = 1) out vec4 outAlbedo;
layout (location = 2) out vec4 outLightPos;

layout (binding = 1) uniform samplerCube samplerShadowCube;

void main() {
	//Normal attachment
	//outNormal = vec4(normalize(fragNormalWorld.xyz), 1.0);

	//Albedo attachment
	outAlbedo = fragColor;

	//Dist to light
	//outVecToLight = vecToLight;

	vec4 vecToLightFrag = vec4(posWorld.xyz - lightPos.xyz, 1.0);
	//vecToLightFrag.x = 2 * vecToLightFrag.x;
	//vecToLightFrag.y = 2 * vecToLightFrag.y;
	//vecToLightFrag = vec4(normalize(vecToLightFrag.xyz), 1.0);
	//float vecToLightLength = length(vecToLight.xyz);

	//Sampled image
	//vec3 vecToSampler = vec3(vecToLight.x, vecToLight.y, vecToLight.z)/vecToLightLength;

	//outVecToLight = vec4(-vecToLight.x, vecToLight.y, vecToLight.z, 1.0);
	//outVecToLight = vec4(texture(samplerShadowCube, vec3(vecToLightFrag.x, -vecToLightFrag.y, vecToLightFrag.z)).rgb, 1.0);
	outVecToLight = posWorld;
	outNormal = texture(samplerShadowCube, vec3(vecToLightFrag.x, -vecToLightFrag.y, vecToLightFrag.z));
}