#pragma once

#include "lve_camera.hpp"
#include "lve_pipeline.hpp"
#include "lve_device.hpp"
#include "lve_game_object.hpp"
#include "lve_allocator.hpp"
#include "lve_frame_info.hpp"

#include <memory>
#include <vector>

namespace lve {
	class SimpleRenderSystem {
	public:
		static constexpr int SIERPINSKI_DEPTH = 3;

		static constexpr int NUMBER_OF_TRIANGLE_VERTICES = 3;

		SimpleRenderSystem(LveDevice &device, VkRenderPass renderPass);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		void renderGameObjects(FrameInfo &frameInfo, std::vector<LveGameObject> &gameObjects);

	private:
		void createPipelineLayout();
		void createPipeline(VkRenderPass renderPass);

		LveDevice &lveDevice;

		LveAllocator lveAllocator{ lveDevice };
		std::unique_ptr<LvePipeline> lvePipeline;
		VkPipelineLayout pipelineLayout;
	};
}