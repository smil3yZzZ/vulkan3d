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
	class ShadowRenderSystem {
	public:
		// Constant depth bias factor (always applied)
		static constexpr float depthBiasConstant = 1.75f;
		// Slope depth bias factor, applied depending on polygon's slope
		static constexpr float depthBiasSlope = 1.5f;

		ShadowRenderSystem(Vk3dDevice& device, VkRenderPass renderPass);
		~ShadowRenderSystem();

		ShadowRenderSystem(const ShadowRenderSystem&) = delete;
		ShadowRenderSystem& operator=(const ShadowRenderSystem&) = delete;

		void renderGameObjects(FrameInfo& frameInfo, glm::mat4 lightProjView);

	private:
		void createShadowPipelineLayout();
		void createShadowPipeline(VkRenderPass renderPass);

		Vk3dDevice& lveDevice;

		Vk3dAllocator lveAllocator{ lveDevice };
		std::unique_ptr<Vk3dPipeline> lveShadowPipeline;
		VkPipelineLayout shadowPipelineLayout;
	};
}