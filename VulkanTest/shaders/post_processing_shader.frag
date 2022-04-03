#version 450

layout (binding = 0) uniform sampler2D uvReflection;
layout (binding = 1) uniform sampler2D lightingMap;
layout(set = 0, binding = 2) uniform PostProcessingBufferUbo {
	vec2 invResolution;
} ubo;

layout (location = 0) out vec4 outColor;

void main() {
	vec2 clipUV = gl_FragCoord.xy * ubo.invResolution;

	vec4 uv = texture(uvReflection, clipUV);
	float alpha = clamp(uv.b, 0, 1);
	
	vec4 color = texture(lightingMap, clipUV);
	vec4 reflectedColor = texture(lightingMap, uv.xy);

	outColor = vec4(mix(color, reflectedColor, alpha).xyz, 1.0);
}