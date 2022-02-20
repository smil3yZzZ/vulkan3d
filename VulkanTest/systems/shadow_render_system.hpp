#pragma once

#include "../lve_camera.hpp"
#include "../lve_pipeline.hpp"
#include "../lve_device.hpp"
#include "../lve_game_object.hpp"
#include "../lve_allocator.hpp"
#include "../lve_frame_info.hpp"

#include <memory>
#include <vector>

namespace lve {
	class ShadowRenderSystem {
	public:
		// Constant depth bias factor (always applied)
		static constexpr float depthBiasConstant = 1.75f;
		// Slope depth bias factor, applied depending on polygon's slope
		static constexpr float depthBiasSlope = 1.5f;

		ShadowRenderSystem(LveDevice& device, VkRenderPass renderPass);
		~ShadowRenderSystem();

		ShadowRenderSystem(const ShadowRenderSystem&) = delete;
		ShadowRenderSystem& operator=(const ShadowRenderSystem&) = delete;

		void renderGameObjects(FrameInfo& frameInfo, glm::mat4 lightProjView);

	private:
		void createShadowPipelineLayout();
		void createShadowPipeline(VkRenderPass renderPass);

		LveDevice& lveDevice;

		LveAllocator lveAllocator{ lveDevice };
		std::unique_ptr<LvePipeline> lveShadowPipeline;
		VkPipelineLayout shadowPipelineLayout;
	};
}