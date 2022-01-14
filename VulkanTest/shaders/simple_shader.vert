#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projectionViewMatrix;
	vec3 directionToLight;
} ubo;


layout(push_constant) uniform Push {
	mat4 modelMatrix; //projection * view * model
	mat4 normalMatrix;
} push;

const float AMBIENT = 0.05;

void main() {
	gl_Position = ubo.projectionViewMatrix * push.modelMatrix * vec4(position, 1.0);

	vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * normal);

	float lightIntensity = AMBIENT + max(dot(normalWorldSpace, ubo.directionToLight), 0);

	fragColor = vec4(lightIntensity * color.xyz, color.a);
}