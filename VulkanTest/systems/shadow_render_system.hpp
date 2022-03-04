#pragma once

#include "../lve_camera.hpp"
#include "../lve_pipeline.hpp"
#include "../lve_device.hpp"
#include "../lve_game_object.hpp"
#include "../lve_allocator.hpp"
#include "../lve_frame_info.hpp"
#include "../lve_renderer.hpp"

#include <memory>
#include <vector>
#include <execution>

namespace lve {
	class ShadowRenderSystem {
	public:
		struct ParallelExecutionInfo {
			uint32_t faceIndex;
			VkCommandBuffer commandBuffer;
			glm::mat4 projectionMatrix;
			glm::mat4 viewMatrix;
			LveRenderer* lveRenderer;
			VkDescriptorSet shadowDescriptorSet;
			LveGameObject::Map& gameObjects;
			ParallelExecutionInfo(uint32_t faceIndex, VkCommandBuffer commandBuffer, glm::mat4 projectionMatrix, glm::mat4 viewMatrix,
				LveRenderer* lveRenderer, VkDescriptorSet shadowDescriptorSet, LveGameObject::Map& gameObjects) :
				faceIndex(faceIndex),
				commandBuffer(commandBuffer),
				projectionMatrix(projectionMatrix),
				viewMatrix(viewMatrix),
				lveRenderer(lveRenderer),
				shadowDescriptorSet(shadowDescriptorSet),
				gameObjects(gameObjects)
			{ }
		};
		// Constant depth bias factor (always applied)
		static constexpr float depthBiasConstant = 1.75f;
		// Slope depth bias factor, applied depending on polygon's slope
		static constexpr float depthBiasSlope = 1.5f;

		ShadowRenderSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout shadowSetLayout);
		~ShadowRenderSystem();

		ShadowRenderSystem(const ShadowRenderSystem&) = delete;
		ShadowRenderSystem& operator=(const ShadowRenderSystem&) = delete;

		void renderGameObjects(ParallelExecutionInfo& parallelExecutionInfo, glm::mat4 lightProjView);
		std::vector<VkCommandBuffer> executeRenderPassCommands(FrameInfo& frameInfo, glm::mat4 projectionMatrix, glm::mat4 viewMatrix, LveRenderer* lveRenderer);
		void copyImageToCube(VkCommandBuffer commandBuffer, uint32_t faceIndex, LveRenderer* lveRenderer);

	private:
		void createShadowPipelineLayout(VkDescriptorSetLayout shadowSetLayout);
		void createShadowPipeline(VkRenderPass renderPass);
		void beginParallelCommandBuffer(VkCommandBuffer commandBuffer);
		void endParallelCommandBuffer(VkCommandBuffer commandBuffer);

		LveDevice& lveDevice;

		LveAllocator lveAllocator{ lveDevice };
		std::unique_ptr<LvePipeline> lveShadowPipeline;
		VkPipelineLayout shadowPipelineLayout;
	};
}