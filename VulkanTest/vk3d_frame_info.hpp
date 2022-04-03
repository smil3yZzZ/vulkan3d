#pragma once

#include "vk3d_camera.hpp"
#include "vk3d_game_object.hpp"

//lib
#include <vulkan/vulkan.h>

namespace vk3d {
	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		Vk3dCamera& camera;
		VkDescriptorSet shadowDescriptorSet;
		VkDescriptorSet mappingsDescriptorSet;
		VkDescriptorSet uvReflectionDescriptorSet;
		VkDescriptorSet gBufferDescriptorSet;
		VkDescriptorSet compositionDescriptorSet;
		VkDescriptorSet postProcessingDescriptorSet;
		Vk3dGameObject::Map& gameObjects;
	};
}