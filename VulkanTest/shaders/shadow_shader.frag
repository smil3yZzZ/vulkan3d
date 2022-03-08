#version 450

layout (location = 0) in float outLength;

layout (location = 0) out float outFragColor;

void main()
{             
    outFragColor = outLength;
}