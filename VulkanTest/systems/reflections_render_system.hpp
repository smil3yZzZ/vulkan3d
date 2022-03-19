#pragma once

#include "../vk3d_camera.hpp"
#include "../vk3d_pipeline.hpp"
#include "../vk3d_device.hpp"
#include "../vk3d_game_object.hpp"
#include "../vk3d_allocator.hpp"
#include "../vk3d_frame_info.hpp"

#include <memory>
#include <vector>

namespace vk3d {
	class ReflectionsRenderSystem {
	public:
		ReflectionsRenderSystem(Vk3dDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout reflectionsSetLayout);
		~ReflectionsRenderSystem();

		ReflectionsRenderSystem(const ReflectionsRenderSystem&) = delete;
		ReflectionsRenderSystem& operator=(const ReflectionsRenderSystem&) = delete;

		void renderGameObjects(FrameInfo& frameInfo);

	private:
		void createReflectionsPipelineLayout(VkDescriptorSetLayout reflectionsSetLayout);
		void createReflectionsPipeline(VkRenderPass renderPass);

		Vk3dDevice& vk3dDevice;

		Vk3dAllocator vk3dAllocator{ vk3dDevice };
		std::unique_ptr<Vk3dPipeline> vk3dReflectionsPipeline;
		VkPipelineLayout reflectionsPipelineLayout;
	};
}