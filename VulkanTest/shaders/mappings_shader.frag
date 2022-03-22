#version 450

layout (location = 0) in vec4 fragMapWorld;
layout (location = 1) in float doNormalize;

layout (location = 0) out vec4 outFragMapWorld;

void main() {
	//Normal attachment
	outFragMapWorld = doNormalize == 1.0 ? vec4(normalize(fragMapWorld.xyz), 1.0) : fragMapWorld;
}