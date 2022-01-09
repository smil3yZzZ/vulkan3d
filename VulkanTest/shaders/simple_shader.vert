#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec4 fragColor;

layout(push_constant) uniform Push {
	mat4 transform; //projection * view * model
	mat4 normalMatrix;
} push;

const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0, -3.0, -1.0));
const float AMBIENT = 0.05;

void main() {
	gl_Position = push.transform * vec4(position, 1.0);

	vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * normal);

	float lightIntensity = AMBIENT + max(dot(normalWorldSpace, DIRECTION_TO_LIGHT), 0);

	fragColor = vec4(lightIntensity * color.xyz, color.a);
}