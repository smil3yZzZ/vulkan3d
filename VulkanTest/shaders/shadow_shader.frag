#version 450

layout (location = 0) in vec4 outPos;
layout (location = 0) out float outFragColor;

layout(set = 0, binding = 0) uniform ShadowUbo {
	float lightFarPlane;
    vec3 lightPosition;
} ubo;

void main()
{             
    // Store distance to light as 32 bit float value
    vec3 lightVec = outPos.xyz - ubo.lightPosition;
    outFragColor = length(lightVec);
}  