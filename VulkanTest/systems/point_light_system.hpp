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
	class PointLightSystem {
	public:
		static constexpr int SIERPINSKI_DEPTH = 3;

		static constexpr int NUMBER_OF_TRIANGLE_VERTICES = 3;

		PointLightSystem(Vk3dDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout gBufferSetLayout, VkDescriptorSetLayout compositionSetLayout);
		~PointLightSystem();

		PointLightSystem(const PointLightSystem&) = delete;
		PointLightSystem& operator=(const PointLightSystem&) = delete;

		void render(FrameInfo& frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout gBufferSetLayout, VkDescriptorSetLayout compositionSetLayout);
		void createPipeline(VkRenderPass renderPass);

		Vk3dDevice& vk3dDevice;

		Vk3dAllocator vk3dAllocator{ vk3dDevice };
		std::unique_ptr<Vk3dPipeline> vk3dPipeline;
		VkPipelineLayout pipelineLayout;
	};
}