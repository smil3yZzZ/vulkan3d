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

	vec4 sum = vec4(0.0);

	sum += texture(lightingMap, uv.xy - 24 * ubo.invResolution) * 0.0162162162;
	sum += texture(lightingMap, uv.xy - 18 * ubo.invResolution) * 0.0540540541;
	sum += texture(lightingMap, uv.xy - 12 * ubo.invResolution) * 0.1216216216;
	sum += texture(lightingMap, uv.xy - 6 * ubo.invResolution) * 0.1945945946;
	
	sum += texture(lightingMap, uv.xy) * 0.2270270270;
	
	sum += texture(lightingMap, uv.xy + 6 * ubo.invResolution) * 0.1945945946;
	sum += texture(lightingMap, uv.xy + 12 * ubo.invResolution) * 0.1216216216;
	sum += texture(lightingMap, uv.xy + 18 * ubo.invResolution) * 0.0540540541;
	sum += texture(lightingMap, uv.xy + 24 * ubo.invResolution) * 0.0162162162;

	outColor = vec4(mix(color, vec4(sum.xyz, 1.0), alpha/2.0).xyz, 1.0);
}