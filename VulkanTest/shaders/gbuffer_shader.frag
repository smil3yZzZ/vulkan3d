#version 450

layout (location = 0) in vec4 fragColor;
layout (location = 1) in vec4 fragPosWorld;
layout (location = 2) in vec4 fragNormalWorld;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;

void main() {
	//Position attachment
	outPosition = fragPosWorld;

	//Normal attachment
	outNormal = vec4(normalize(fragNormalWorld.xyz), 1.0);

	//Albedo attachment
	outAlbedo = fragColor;
}