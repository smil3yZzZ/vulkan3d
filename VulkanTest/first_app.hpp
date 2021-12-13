#pragma once

#include "lve_window.hpp"
#include "lve_device.hpp"
#include "lve_game_object.hpp"
#include "lve_allocator.hpp"
#include "lve_renderer.hpp"

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

		LveWindow lveWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
		LveDevice lveDevice{ lveWindow };
		LveRenderer lveRenderer{ lveWindow, lveDevice};

		LveAllocator lveAllocator{ lveDevice };
		std::vector<LveGameObject> gameObjects;
		std::vector<std::shared_ptr<LveModel>> gameModels;
	};
}