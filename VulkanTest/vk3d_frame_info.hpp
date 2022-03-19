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
		VkDescriptorSet reflectionsDescriptorSet;
		VkDescriptorSet gBufferDescriptorSet;
		VkDescriptorSet compositionDescriptorSet;
		Vk3dGameObject::Map& gameObjects;
	};
}