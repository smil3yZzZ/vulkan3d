#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

layout(location = 0) out vec4 fragColor;

layout(push_constant) uniform Push {
	mat4 transform;
	vec4 color;
} push;

void main() {
	gl_Position = push.transform * vec4(position, 1.0);
	fragColor = color;
}