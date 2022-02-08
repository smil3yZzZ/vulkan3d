#version 450

layout(location = 0) in vec3 position;

layout(push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 lightProjView;
} push;

void main() 
{
	vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);
	gl_Position = push.lightProjView * positionWorld;
}