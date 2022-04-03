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
	class SceneRenderSystem {
	public:
		static constexpr int SIERPINSKI_DEPTH = 3;

		static constexpr int NUMBER_OF_TRIANGLE_VERTICES = 3;

		SceneRenderSystem(Vk3dDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout gBufferSetLayout, 
			VkDescriptorSetLayout compositionSetLayout, VkRenderPass postProcessingRenderPass, VkDescriptorSetLayout postProcessingSetLayout);
		~SceneRenderSystem();

		SceneRenderSystem(const SceneRenderSystem&) = delete;
		SceneRenderSystem& operator=(const SceneRenderSystem&) = delete;

		void renderGameObjectsLighting(FrameInfo& frameInfo, glm::mat4 invViewProj, glm::vec2 invResolution);
		void renderPostProcessing(FrameInfo& frameInfo);

	private:
		void createGBufferPipelineLayout(VkDescriptorSetLayout gBufferLayout);
		void createGBufferPipeline(VkRenderPass lightingRenderPass);
		void createCompositionPipelineLayout(VkDescriptorSetLayout compositionSetLayout);
		void createCompositionPipeline(VkRenderPass lightingRenderPass);
		void createPostProcessingPipelineLayout(VkDescriptorSetLayout postProcessingSetLayout);
		void createPostProcessingPipeline(VkRenderPass postProcessingRenderPass);

		Vk3dDevice &vk3dDevice;

		Vk3dAllocator vk3dAllocator{ vk3dDevice };
		std::unique_ptr<Vk3dPipeline> vk3dGBufferPipeline;
		VkPipelineLayout gBufferPipelineLayout;
		std::unique_ptr<Vk3dPipeline> vk3dCompositionPipeline;
		VkPipelineLayout compositionPipelineLayout;
		std::unique_ptr<Vk3dPipeline> vk3dPostProcessingPipeline;
		VkPipelineLayout postProcessingPipelineLayout;
	};
}