#pragma once

#include "lve_window.hpp"
#include "lve_pipeline.hpp"
#include "lve_device.hpp"
#include "lve_swap_chain.hpp"
#include "lve_game_object.hpp"
#include "lve_allocator.hpp"

#include <memory>
#include <vector>

namespace lve {
	class FirstApp {
	public:
		static constexpr int SIERPINSKI_DEPTH = 3;

		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;
		static constexpr int NUMBER_OF_TRIANGLE_VERTICES = 3;

		FirstApp();
		~FirstApp();

		FirstApp(const FirstApp&) = delete;
		FirstApp& operator=(const FirstApp&) = delete;

		void run();
	private:
		void loadGameObjects();
		void updateModels(int powIteration);
		void createPipelineLayout();
		void createPipeline();
		void createCommandBuffers();
		void freeCommandBuffers();
		void drawFrame();
		void recreateSwapChain();
		void recordCommandBuffer(size_t vertexBufferIndex, int imageIndex);
		void renderGameObjects(size_t vertexBufferIndex, VkCommandBuffer commandBuffer);

		LveWindow lveWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
		LveDevice lveDevice{ lveWindow };
		LveAllocator lveAllocator{ lveDevice };
		std::unique_ptr<LveSwapChain> lveSwapChain;
		std::unique_ptr<LvePipeline> lvePipeline;
		VkPipelineLayout pipelineLayout;
		std::vector<VkCommandBuffer> commandBuffers;
		std::vector<LveGameObject> gameObjects;
		std::vector<LveModel::Vertex> vertices{};
	};
}