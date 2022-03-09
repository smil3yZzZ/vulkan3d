#version 450

layout (location = 0) in vec3 worldPos;
layout (location = 1) in vec3 lightPos;

layout (location = 0) out float outFragColor;
//layout (location = 0) out vec4 outWorldPos;

void main()
{             
    outFragColor = length(worldPos - lightPos);
    //outFragColor = outLength;
    //outWorldPos = vec4(worldPos, 1.0);
}