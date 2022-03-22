#version 450

layout (input_attachment_index = 0, binding = 0) uniform subpassInput samplerNormal;
layout (input_attachment_index = 1, binding = 1) uniform subpassInput samplerAlbedo;
layout (input_attachment_index = 2, binding = 2) uniform subpassInput samplerPositionDepth;
layout (binding = 4) uniform samplerCube samplerShadowCube;
layout (binding = 5) uniform sampler2DArray samplerMappingsMap;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 3) uniform CompositionUbo {
	vec3 viewPos;
	vec4 ambientLightColor; //w is intensity
	vec3 lightPosition;
	vec4 lightColor; // w is light intensity
} ubo;

layout(push_constant) uniform Push {
	mat4 invViewProj;
	vec2 invResolution;
} push;

const float specularStrength = 8;
const float shininess = 32;
const float EPSILON = 0.15;

float maxDistance = 15;
float resolution  = 0.3;
int   steps       = 10;
float thickness   = 0.5;

/*
float unidirectionalShadowCalculation(vec3 lightProjCoords)
{
	// If the fragment is outside the light's projection then it is outside
	// the light's influence, which means it is in the shadow (notice that
	// such sample would be outside the shadow map image)
	
	if (abs(lightProjCoords.x) > 1.0 ||
		abs(lightProjCoords.y) > 1.0 ||
		abs(lightProjCoords.z) > 1.0)
		return 0.5;

	// transform to [0,1] range
    lightProjCoords.xy = lightProjCoords.xy * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(samplerShadowDepth, lightProjCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = lightProjCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth > closestDepth ? 0.5 : 0.0;

    return shadow;
}
*/

void main() {
	
	// Read previous pass shadow depth & G-Buffer values from previous sub pass
	vec2 clipUV = gl_FragCoord.xy * push.invResolution;
	vec2 clipXY = clipUV * 2.0 - 1.0;

	// Screen-space mappings start
	/*
	//Mappings variables
	vec4 positionFrom = texture(samplerMappingsMap, vec3(clipUV, 0));
	vec3 unitPositionFrom = normalize(positionFrom.xyz);
	vec3 reflectionNormal = normalize(texture(samplerMappingsMap, vec3(clipUV, 1)).xyz);
	vec3 pivot = normalize(reflect(unitPositionFrom, reflectionNormal));

	vec4 startView = vec4(positionFrom.xyz + (pivot * 0), 1.0);
	vec4 endView = vec4(positionFrom.xyz + (pivot * maxDistance), 1.0);

	vec4 startFrag = startView;
	// Project to screen space.
	startFrag = ubo.projection * startFrag;
	// Perform the perspective divide.
	startFrag.xyz /= startFrag.w;
	// Convert the screen-space XY coordinates to UV coordinates.
	startFrag.xy = startFrag.xy * 0.5 + 0.5;
	// Convert the UV coordinates to fragment/pixel coordnates.
	startFrag.xy /= push.invResolution;

	vec4 endFrag = endView;
    endFrag = ubo.projection * endFrag;
    endFrag.xyz /= endFrag.w;
    endFrag.xy = endFrag.xy * 0.5 + 0.5;
    endFrag.xy /= push.invResolution;
	*/
	// Screen-space mappings end

	vec4 clipScene = vec4(clipXY, subpassLoad(samplerPositionDepth).x, 1.0);

	vec4 fragPosWorld_w = push.invViewProj * clipScene;
	vec3 fragPosWorld = fragPosWorld_w.xyz / fragPosWorld_w.w;

	//Calculate shadow
	vec3 inDirToLight = fragPosWorld - ubo.lightPosition;
	float dist    = length(inDirToLight);

	float depth = texture(samplerShadowCube, vec3(inDirToLight.x, -inDirToLight.y, inDirToLight.z)).r;
	float shadow = dist < (depth + EPSILON) ? 0.0 : 0.5;

	//vec4 fragPosLight_w = ubo.lightProjView * fragPosWorld_w;
	//vec3 fragPosLight = fragPosLight_w.xyz / fragPosLight_w.w;
	//float directionalShadow = unidirectionalShadowCalculation(fragPosLight);

	vec3 normal = subpassLoad(samplerNormal).xyz;
	vec4 fragColor = subpassLoad(samplerAlbedo);

	vec3 directionToView = normalize(ubo.viewPos - fragPosWorld);
	vec3 directionToLight = normalize(-inDirToLight);
	vec3 halfwayDirection = normalize(directionToLight + directionToView);

	float attenuation = 1.0 / dot(-inDirToLight, -inDirToLight); // distance squared

	vec3 directionToReflection = reflect(-directionToLight, normal);  

	vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
	vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 diffuseLight = lightColor * max(dot(normal, normalize(directionToLight)), 0);

	float spec = pow(max(dot(normal, halfwayDirection), 0.0), shininess);
	vec3 specularLight = specularStrength * spec * lightColor;
	
	outColor = vec4((diffuseLight + (1.0 - shadow) * ambientLight + specularLight) * fragColor.xyz, fragColor.a);
}