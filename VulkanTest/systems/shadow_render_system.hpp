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
		static constexpr float depthBiasConstant = 0.75f;
		// Slope depth bias factor, applied depending on polygon's slope
		static constexpr float depthBiasSlope = 0.25f;

		ShadowRenderSystem(Vk3dDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout shadowSetLayout);
		~ShadowRenderSystem();

		ShadowRenderSystem(const ShadowRenderSystem&) = delete;
		ShadowRenderSystem& operator=(const ShadowRenderSystem&) = delete;

		void renderGameObjects(FrameInfo& frameInfo);

	private:
		void createShadowPipelineLayout(VkDescriptorSetLayout shadowSetLayout);
		void createShadowPipeline(VkRenderPass renderPass);

		Vk3dDevice& vk3dDevice;

		Vk3dAllocator vk3dAllocator{ vk3dDevice };
		std::unique_ptr<Vk3dPipeline> vk3dShadowPipeline;
		VkPipelineLayout shadowPipelineLayout;
	};
}