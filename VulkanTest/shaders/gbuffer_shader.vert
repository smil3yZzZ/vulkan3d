#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 fragNormalWorld;

layout(set = 0, binding = 0) uniform GBufferUbo {
	mat4 projection;
	mat4 view;
} ubo;

layout(push_constant) uniform Push {
	mat4 modelMatrix; //projection * view * model
	mat4 normalMatrix;
} push;

void main() {
	vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);

	gl_Position = ubo.projection * ubo.view * positionWorld;

	fragNormalWorld = vec4(normalize(mat3(push.normalMatrix) * normal), 1.0);
	fragColor = color;
}