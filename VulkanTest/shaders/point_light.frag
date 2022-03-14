#version 450

layout (location = 0) in vec2 fragOffset;
layout (location = 0) out vec4 outColor;

layout(set = 1, binding = 4) uniform CompositionUbo {
	vec3 viewPos;
	vec4 ambientLightColor; //w is intensity
	vec3 lightPosition;
	vec4 lightColor; // w is light intensity
} compositionUbo;

void main() {
	float dis = sqrt(dot(fragOffset, fragOffset));
	if (dis >= 1.0) {
		discard;
	}
	outColor = vec4(compositionUbo.lightColor.xyz, 1.0);
}
