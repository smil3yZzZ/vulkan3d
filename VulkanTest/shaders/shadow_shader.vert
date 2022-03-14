#version 450

#extension GL_EXT_multiview : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out float outLength;
layout(location = 1) out vec3 worldPos;
layout(location = 2) out vec3 lightPos;

layout(push_constant) uniform Push {
	mat4 modelMatrix;
} push;

layout(set = 0, binding = 0) uniform ShadowUbo {
	mat4 lightProjectionView[6];
    vec3 lightPosition;
} ubo;

const vec2 QUAD_VERTICES[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);

//Use depth buffer

void main() 
{
	vec4 inPos = vec4(position, 1.0);
	vec4 positionWorld = push.modelMatrix * inPos;
	gl_Position = ubo.lightProjectionView[gl_ViewIndex] * positionWorld;
	outLength = length(positionWorld.xyz - ubo.lightPosition.xyz);
	worldPos = positionWorld.xyz;
	//lightPos = ubo.lightPosition.xyz;
	//gl_Position = vec4(QUAD_VERTICES[gl_VertexIndex], 1.0, 1.0);
}