#version 450

layout (location = 0) in vec4 fragNormalWorld;

layout (location = 0) out vec4 outNormal;

void main() {
	//Normal attachment
	outNormal = vec4(normalize(fragNormalWorld.xyz), 1.0);
}