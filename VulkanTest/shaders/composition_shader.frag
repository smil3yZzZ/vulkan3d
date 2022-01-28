#version 450

layout (input_attachment_index = 0, binding = 0) uniform subpassInput samplerPosition;
layout (input_attachment_index = 1, binding = 1) uniform subpassInput samplerNormal;
layout (input_attachment_index = 2, binding = 2) uniform subpassInput samplerAlbedo;
layout (input_attachment_index = 3, binding = 3) uniform subpassInput samplerPositionDepth;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 4) uniform GlobalUbo {
	vec3 viewPos;
	vec4 ambientLightColor; //w is intensity
	vec3 lightPosition;
	vec4 lightColor; // w is light intensity
} ubo;

layout(push_constant) uniform Push {
	mat4 invViewProj;
	vec2 invResolution;
} push;

const float specularStrength = 0.5;
const float shininess = 1;

void main() {
	// Read G-Buffer values from previous sub pass
	vec4 clip = vec4(gl_FragCoord.xy * push.invResolution * 2.0 - 1.0, subpassLoad(samplerPositionDepth).x, 1.0);
	vec4 fragPosWorld_w = push.invViewProj * clip;
	vec3 fragPosWorld = fragPosWorld_w.xyz / fragPosWorld_w.w;

	vec3 normal = subpassLoad(samplerNormal).xyz;
	vec4 fragColor = subpassLoad(samplerAlbedo);

	vec3 directionToView = normalize(ubo.viewPos - fragPosWorld);
	vec3 directionToLight = ubo.lightPosition - fragPosWorld;
	vec3 halfwayDirection = normalize(directionToLight + directionToView);

	float attenuation = 1.0 / dot(directionToLight, directionToLight); // distance squared

	vec3 directionToReflection = reflect(-directionToLight, normal);  

	vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
	vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 diffuseLight = lightColor * max(dot(normal, normalize(directionToLight)), 0);

	float spec = pow(max(dot(normal, halfwayDirection), 0.0), shininess);
	vec3 specularLight = specularStrength * spec * lightColor;
	
	outColor = vec4((diffuseLight + ambientLight + specularLight) * fragColor.xyz, fragColor.a);
	
}