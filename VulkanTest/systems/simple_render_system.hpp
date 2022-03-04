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
	class SimpleRenderSystem {
	public:
		static constexpr int SIERPINSKI_DEPTH = 3;

		static constexpr int NUMBER_OF_TRIANGLE_VERTICES = 3;

		SimpleRenderSystem(Vk3dDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout gBufferSetLayout, VkDescriptorSetLayout compositionSetLayout);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		void renderGameObjects(FrameInfo& frameInfo, glm::mat4 invViewProj, glm::vec2 invResolution);

	private:
		void createGBufferPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createGBufferPipeline(VkRenderPass renderPass);
		void createCompositionPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createCompositionPipeline(VkRenderPass renderPass);

		Vk3dDevice &lveDevice;

		Vk3dAllocator lveAllocator{ lveDevice };
		std::unique_ptr<Vk3dPipeline> lveGBufferPipeline;
		VkPipelineLayout gBufferPipelineLayout;
		std::unique_ptr<Vk3dPipeline> lveCompositionPipeline;
		VkPipelineLayout compositionPipelineLayout;
	};
}