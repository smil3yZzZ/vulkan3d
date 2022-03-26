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
	class ReflectionRenderSystem {
	public:
		ReflectionRenderSystem(Vk3dDevice& device, VkRenderPass mappingsRenderPass, VkDescriptorSetLayout mappingsSetLayout, VkRenderPass uvReflectionMapRenderPass, VkDescriptorSetLayout uvReflectionMapSetLayout);
		~ReflectionRenderSystem();

		ReflectionRenderSystem(const ReflectionRenderSystem&) = delete;
		ReflectionRenderSystem& operator=(const ReflectionRenderSystem&) = delete;

		void renderMappings(FrameInfo& frameInfo);
		void renderUVReflectionMap(FrameInfo& frameInfo);

	private:
		void createMappingsPipelineLayout(VkDescriptorSetLayout mappingsSetLayout);
		void createMappingsPipeline(VkRenderPass mappingsRenderPass);
		void createUVReflectionMapPipelineLayout(VkDescriptorSetLayout uvReflectionMapSetLayout);
		void createUVReflectionMapPipeline(VkRenderPass uvReflectionMapRenderPass);

		Vk3dDevice& vk3dDevice;

		Vk3dAllocator vk3dAllocator{ vk3dDevice };
		std::unique_ptr<Vk3dPipeline> vk3dMappingsPipeline;
		VkPipelineLayout mappingsPipelineLayout;
		std::unique_ptr<Vk3dPipeline> vk3dUVReflectionMapPipeline;
		VkPipelineLayout uvReflectionMapPipelineLayout;
	};
}