#pragma once

#include "lve_window.hpp"
#include "lve_pipeline.hpp"
#include "lve_device.hpp"
#include "lve_swap_chain.hpp"
#include "lve_model.hpp"
#include "lve_allocator.hpp"

#include <memory>
#include <vector>

namespace lve {
	class FirstApp {
	public:
		static constexpr int SIERPINSKI_DEPTH = 10;

		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;
		static constexpr int NUMBER_OF_TRIANGLE_VERTICES = 3;

		FirstApp();
		~FirstApp();

		FirstApp(const FirstApp&) = delete;
		FirstApp& operator=(const FirstApp&) = delete;

		void run();
	private:
		void loadModels();
		void updateModels(int powIteration);
		void createPipelineLayout();
		void createPipeline();
		void initCommandBuffers();
		void createCommandBuffer(size_t vertexBufferIndex);
		void drawFrame();

		LveWindow lveWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
		LveDevice lveDevice{ lveWindow };
		LveAllocator lveAllocator{ lveDevice };
		LveSwapChain lveSwapChain{ lveDevice, lveWindow.getExtent() };
		std::unique_ptr<LvePipeline> lvePipeline;
		VkPipelineLayout pipelineLayout;
		std::vector<VkCommandBuffer> commandBuffers;
		std::unique_ptr<LveModel> lveModel;
		std::vector<LveModel::Vertex> vertices{};
		size_t currentIndex = 0;

	};
}