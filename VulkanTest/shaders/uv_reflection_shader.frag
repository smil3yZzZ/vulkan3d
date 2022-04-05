#version 450

layout(set = 0, binding = 0) uniform UVReflectionBufferUbo {
	vec3 viewPos;
	mat4 projection;
	mat4 view;
	vec2 invResolution;
} ubo;
layout (binding = 1) uniform sampler2DArray samplerMappingsMap;

layout(push_constant) uniform Push {
	mat4 modelMatrix;
	float reflection;
} push;

layout (location = 0) out vec4 outUVReflection;

float depthCheckBias = 0.02;
float loops = 100.0;

// Length per ray marching iteration
float marchLength = 0.04;

struct RayMarch
{
	bool hit;
	vec2 uv;
};

void main() {
	vec4 uv = vec4(0.0);

	if (push.reflection != 1.0
     ) { outUVReflection = uv; return; }

	// Compute current clip fragment
	vec2 clipUV = gl_FragCoord.xy * ubo.invResolution;
	vec2 clipXY = clipUV * 2.0 - 1.0;

	//Mappings variables
	vec4 positionFrom = texture(samplerMappingsMap, vec3(clipUV, 0));
	vec3 unitPositionFrom = normalize(positionFrom.xyz);
	vec3 reflectionNormal = normalize(texture(samplerMappingsMap, vec3(clipUV, 1)).xyz);
	vec3 pivot = normalize(reflect(unitPositionFrom, reflectionNormal));

	// The Current Position in 3D
	vec4 curPos = vec4(positionFrom.xyz, 1.0);
 
	// The Current UV
	vec4 curUV = vec4(0.0, 0.0, 0.0, 1.0);
	
	RayMarch ray = {false, vec2(0.0, 0.0)};

	float i;

	float maxDistance = length(vec3(curPos.xyz + pivot.xyz * marchLength * loops - curPos.xyz));

	for (i = 1.0; i < loops; i++)
    {
		// Has it hit anything yet
        if (ray.hit == false)
        {
			// Update the Current Position of the Ray
			curPos = vec4(curPos.xyz + pivot.xyz * marchLength, 1.0);

			// Project to screen space.
			vec4 curFrag = ubo.projection * curPos;
			// Perform the perspective divide.
			curFrag.xyz /= curFrag.w;
			// Convert the screen-space XY coordinates to UV coordinates.
			curFrag.xy = curFrag.xy * 0.5 + 0.5;
			curUV.xyz = vec3(curFrag.xy, curPos.z);
			// Convert the UV coordinates to fragment/pixel coordinates.
			//startFrag.xy /= ubo.invResolution;

			// The Depth of the Current Pixel
			float curDepth = texture(samplerMappingsMap, vec3(curUV.xy, 0)).z;

			if (abs(curUV.z - curDepth) < depthCheckBias)
            {
                // If it's hit something, then return the UV position
                ray.hit = true;
                ray.uv = curUV.xy;
                break;
            }

		}
	}

	float amount = 1.0;

	if (ray.hit == true) {
		// Fade considering distance, remove reflections out of texture
		amount *= 1.0 - length(curPos.xyz - positionFrom.xyz)/maxDistance;
		amount *= (ray.uv.x < 0 || ray.uv.x > 1 ? 0 : 1) * (ray.uv.y < 0 || ray.uv.y > 1 ? 0 : 1);
		uv = vec4(ray.uv.xy, amount, amount);
	}

	outUVReflection = uv;
}