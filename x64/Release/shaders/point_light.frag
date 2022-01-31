#version 450

layout (location = 0) in vec2 fragOffset;
layout (location = 0) out vec4 outColor;

<<<<<<< HEAD
layout(set = 0, binding = 0) uniform GlobalUbo {
	vec3 viewPos;
	mat4 projection;
	mat4 view;
	vec4 ambientLightColor; //w is intensity
	vec3 lightPosition;
	vec4 lightColor; // w is light intensity
} ubo;
=======
layout(set = 1, binding = 3) uniform CompositionUbo {
	vec3 viewPos;
	vec4 ambientLightColor; //w is intensity
	vec3 lightPosition;
	vec4 lightColor; // w is light intensity
} compositionUbo;
>>>>>>> feature/deferred_rendering

void main() {
	float dis = sqrt(dot(fragOffset, fragOffset));
	if (dis >= 1.0) {
		discard;
	}
<<<<<<< HEAD
	outColor = vec4(ubo.lightColor.xyz, 1.0);
=======
	outColor = vec4(compositionUbo.lightColor.xyz, 1.0);
>>>>>>> feature/deferred_rendering
}
