#pragma once

#include "vk3d_window.hpp"
#include "vk3d_device.hpp"
#include "vk3d_game_object.hpp"
#include "vk3d_allocator.hpp"
#include "vk3d_renderer.hpp"
#include "vk3d_swap_chain.hpp"

#include <memory>
#include <vector>

namespace vk3d {
	class Vk3dApp {
	public:
		static constexpr int SIERPINSKI_DEPTH = 3;

		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;
		static constexpr int NUMBER_OF_TRIANGLE_VERTICES = 3;

		static constexpr float MIN_SECONDS_PER_FRAME = 1.f/60.f;

		static constexpr float LIGHT_NEAR_PLANE = 0.1f;
		static constexpr float LIGHT_FAR_PLANE = 50.0f;

		static constexpr float CAMERA_NEAR_PLANE = 0.1f;
		static constexpr float CAMERA_FAR_PLANE = 50.0f;

		Vk3dApp();
		~Vk3dApp();

		Vk3dApp(const Vk3dApp&) = delete;
		Vk3dApp& operator=(const Vk3dApp&) = delete;

		void run();
	private:
		void loadGameObjects();
		void updateModels(int powIteration);

		Vk3dWindow vk3dWindow{WIDTH, HEIGHT, "Vulkan3d App"};
		Vk3dDevice vk3dDevice{ vk3dWindow };
		Vk3dAllocator vk3dAllocator{ vk3dDevice };
		Vk3dRenderer vk3dRenderer{ vk3dWindow, vk3dDevice, vk3dAllocator};

		// note: order of declarations matters

		Vk3dGameObject::Map gameObjects;
		std::vector<std::shared_ptr<Vk3dModel>> gameModels;
	};
}