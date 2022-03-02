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

		static constexpr float CAMERA_NEAR = 0.1f;
		static constexpr float CAMERA_FAR = 50.0f;

		static constexpr float LIGHT_NEAR = 0.1f;
		static constexpr float LIGHT_FAR = 24.0f;


		static constexpr float MIN_SECONDS_PER_FRAME = 1.f/60.f;

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
		LveAllocator lveAllocator{ lveDevice };
		LveRenderer lveRenderer{ lveWindow, lveDevice, lveAllocator};

		// note: order of declarations matters

		LveGameObject::Map gameObjects;
		std::vector<std::shared_ptr<LveModel>> gameModels;
	};
}