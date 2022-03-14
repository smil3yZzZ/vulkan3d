#version 450

layout (location = 0) in float outLength;
layout (location = 1) in vec3 worldPos;
layout (location = 2) in vec3 lightPos;

//layout (location = 0) out float outFragColor;
layout (location = 0) out vec4 outWorldPos;

void main()
{             
    //outFragColor = length(lightPos.xyz - worldPos.xyz);
    //outFragColor = outLength;
    outWorldPos = vec4(worldPos, 1.0);
}