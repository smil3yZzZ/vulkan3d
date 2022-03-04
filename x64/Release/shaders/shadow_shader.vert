#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec4 outPos;

layout(push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 lightProjView;
} push;

void main() 
{
	vec4 inPos = vec4(position, 1.0);
	vec4 positionWorld = push.modelMatrix * inPos;
	gl_Position = push.lightProjView * positionWorld;
	outPos = inPos;
}