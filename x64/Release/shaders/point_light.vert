#version 450

const vec2 OFFSETS[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);

layout (location = 0) out vec2 fragOffset;

layout(set = 0, binding = 0) uniform GBufferUbo {
	mat4 projection;
	mat4 view;
} gBufferUbo;

layout(set = 1, binding = 3) uniform CompositionUbo {
	vec3 viewPos;
	vec4 ambientLightColor; //w is intensity
	vec3 lightPosition;
	vec4 lightColor; // w is light intensity
} compositionUbo;

const float LIGHT_RADIUS = 0.1;

void main() {
	fragOffset = OFFSETS[gl_VertexIndex];
	vec3 cameraRightWorld = {gBufferUbo.view[0][0], gBufferUbo.view[1][0], gBufferUbo.view[2][0]};
	vec3 cameraUpWorld = {gBufferUbo.view[0][1], gBufferUbo.view[1][1], gBufferUbo.view[2][1]};

	vec3 positionWorld = compositionUbo.lightPosition.xyz
	+ LIGHT_RADIUS * fragOffset.x * cameraRightWorld
	+ LIGHT_RADIUS * fragOffset.y * cameraUpWorld;

	gl_Position = gBufferUbo.projection * gBufferUbo.view * vec4(positionWorld, 1.0);
}