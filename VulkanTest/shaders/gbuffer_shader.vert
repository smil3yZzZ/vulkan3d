#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 fragNormalWorld;
layout (location = 2) out vec4 vecToLight;
layout (location = 3) out float vecToLightLength;
layout (location = 4) out vec3 lightPos;
layout (location = 5) out vec4 posWorld;

layout(set = 0, binding = 0) uniform GBufferUbo {
	mat4 projection;
	mat4 view;
	vec3 lightPosition;
} ubo;

layout(push_constant) uniform Push {
	mat4 modelMatrix; //projection * view * model
	mat4 normalMatrix;
} push;

void main() {
	vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);

	gl_Position = ubo.projection * ubo.view * positionWorld;

	fragColor = color;
	fragNormalWorld = vec4(normalize(mat3(push.normalMatrix) * normal), 1.0);

	vecToLight = vec4(positionWorld.xyz - ubo.lightPosition.xyz, 1.0);

	vecToLightLength = length(vecToLight.xyz);

	lightPos = ubo.lightPosition.xyz;

	posWorld = positionWorld;

	vecToLight = normalize(vec4(vecToLight.x, vecToLight.y, vecToLight.z, 1.0));
}