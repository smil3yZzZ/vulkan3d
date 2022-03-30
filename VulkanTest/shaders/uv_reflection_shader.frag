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

//float resolution  = 0.05;
//int   steps       = 5;
//float thickness   = 0.5;

float depthCheckBias = 0.07;
float loops = 40.0;

// Length per ray marching iteration
float marchLength = 0.1;

struct RayTraceOutput
{
	bool Hit;
	vec2 UV;
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
	vec3 unitPositionFrom = normalize(positionFrom.xyz - ubo.viewPos);
	vec3 reflectionNormal = normalize(texture(samplerMappingsMap, vec3(clipUV, 1)).xyz);
	vec3 pivot = normalize(reflect(unitPositionFrom, reflectionNormal));

	// The Current Position in 3D
	vec4 curPos = vec4(positionFrom.xyz, 1.0);
 
	// The Current UV
	vec4 curUV = vec4(0.0, 0.0, 0.0, 1.0);
	
	RayTraceOutput ray = {false, vec2(0.0, 0.0)};

	float i;

	for (i = 1.0; i < loops; i++)
    {
		// Has it hit anything yet
        if (ray.Hit == false)
        {
			// Update the Current Position of the Ray
			curPos = vec4(curPos.xyz + pivot.xyz * marchLength, 1.0);

			// Project to screen space.
			vec4 curFrag = ubo.projection * ubo.view * curPos;
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
                ray.Hit = true;
                ray.UV = curUV.xy;
                break;
            }

			/*
			for (int i = 0; i < SAMPLE_COUNT; i++)
            {
                if (abs(curUV.z - curDepth) < depthCheckBias)
                {
                    // If it's hit something, then return the UV position
                    ray.Hit = true;
                    ray.UV = curUV.xy;
                    break;
                }
                curDepth = GetDepth(curUV .xy + (RAND_SAMPLES[i].xy * HalfPixel * 2));
            }
			*/

		}
	}

	float amount = 1.0;

	if (ray.Hit == true) {
		// Fade at edges
		//if (ray.UV.y < EdgeCutOff * 2) amount *= (ray.UV.y / EdgeCutOff / 2);
		uv = vec4(ray.UV.xy, amount, amount);
	}

	/*
	//Last checks and set reflection!
	
	float visibility = hit1 * positionTo.w 
	                        * (1 - max(dot(-unitPositionFrom, pivot), 0))
						    * (1 - clamp(depth / thickness, 0, 1))
						    * (1 - clamp(length(positionTo - positionFrom)/maxDistance, 0, 1))
							* (uv.x < 0 || uv.x > 1 ? 0 : 1)
							* (uv.y < 0 || uv.y > 1 ? 0 : 1);

	visibility = clamp(visibility, 0, 1);

	uv.ba = vec2(visibility);
	*/

	outUVReflection = curPos;
	//outUVReflection = vec4(ray.Hit, i, 0, 0);
}