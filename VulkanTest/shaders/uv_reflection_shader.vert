#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(set = 0, binding = 0) uniform UVReflectionBufferUbo {
	mat4 projection;
	mat4 view;
	vec2 invResolution;
} ubo;

layout(push_constant) uniform Push {
	mat4 modelMatrix;
	float reflection;
} push;

void main() {
	vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);
	gl_Position = ubo.projection * ubo.view * positionWorld;
}