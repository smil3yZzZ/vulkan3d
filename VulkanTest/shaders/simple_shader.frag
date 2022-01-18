#version 450

layout (location = 0) in vec4 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
	vec3 viewPos;
	mat4 projection;
	mat4 view;
	vec4 ambientLightColor; //w is intensity
	vec3 lightPosition;
	vec4 lightColor; // w is light intensity
} ubo;

layout(push_constant) uniform Push {
	mat4 modelMatrix; //projection * view * model
	mat4 normalMatrix;
} push;

const float specularStrength = 0.5;
const float shininess = 1;

void main() {
	vec3 directionToView = normalize(ubo.viewPos - fragPosWorld);
	vec3 directionToLight = ubo.lightPosition - fragPosWorld;
	vec3 halfwayDirection = normalize(directionToLight + directionToView);

	float attenuation = 1.0 / dot(directionToLight, directionToLight); // distance squared

	vec3 normal = normalize(fragNormalWorld);
	vec3 directionToReflection = reflect(-directionToLight, normal);  

	vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
	vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 diffuseLight = lightColor * max(dot(normal, normalize(directionToLight)), 0);

	float spec = pow(max(dot(normal, halfwayDirection), 0.0), shininess);
	vec3 specularLight = specularStrength * spec * lightColor;

	outColor = vec4((diffuseLight + ambientLight + specularLight) * fragColor.xyz, fragColor.a);
}