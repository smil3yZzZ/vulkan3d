#version 450

layout (location = 0) in vec3 worldPos;
layout (location = 1) in vec3 lightPos;

layout (location = 0) out float outFragColor;

void main()
{             
    outFragColor = length(worldPos.xyz - lightPos.xyz);
}