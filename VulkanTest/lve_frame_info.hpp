#pragma once

#include "lve_camera.hpp"
#include "lve_game_object.hpp"

//lib
#include <vulkan/vulkan.h>

namespace lve {
	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		LveCamera& camera;
		VkDescriptorSet shadowDescriptorSet;
		VkDescriptorSet gBufferDescriptorSet;
		VkDescriptorSet compositionDescriptorSet;
		LveGameObject::Map& gameObjects;
	};
}