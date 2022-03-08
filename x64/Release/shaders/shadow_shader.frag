#version 450

layout (location = 0) in vec4 outPos;
layout (location = 1) in vec3 lightPosition;

layout (location = 0) out float outFragColor;

void main()
{             
    // Store distance to light as 32 bit float value
    vec3 lightVec = outPos.xyz - lightPosition;
    outFragColor = length(lightVec);
    //outFragColor = 0.5;
}