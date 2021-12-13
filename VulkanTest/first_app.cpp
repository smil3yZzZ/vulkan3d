#include "first_app.hpp"
#include "simple_render_system.hpp"

#include <math.h>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>

#include <iostream>


namespace lve {

	FirstApp::FirstApp() {
		loadGameObjects();
	}

	FirstApp::~FirstApp() {
	}

	void FirstApp::run() {
		SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass()};
		int frame = 0;
		while (!lveWindow.shouldClose()) {
			glfwPollEvents();
			if (auto commandBuffer = lveRenderer.beginFrame()) {
				lveRenderer.beginSwapChainRenderPass(commandBuffer);
				if (frame == 60) {
					gameObjects.erase(gameObjects.begin());
				}
				simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
				lveRenderer.endSwapChainRenderPass(commandBuffer);
				lveRenderer.endFrame();
				frame++;
			}
		}
		vkDeviceWaitIdle(lveDevice.device());
	}

	void FirstApp::loadGameObjects() {
		std::vector<LveModel::Vertex> triangleVertices = {
			{{0.0f, -0.5f}, {1.0f, 1.0f, 0.0f, 1.0f}},
			{{0.5f, 0.5f}, {1.0f, 1.0f, 0.0f, 1.0f}},
			{{-0.5f, 0.5f}, {1.0f, 1.0f, 0.0f, 1.0f}}
		};

		auto triangleModel = std::make_shared<LveModel>(lveDevice, triangleVertices, lveAllocator);

		gameModels.push_back(std::move(triangleModel));

		auto triangle = LveGameObject::createGameObject();
		triangle.model = gameModels.back();
		triangle.color = {.1f, .8f, .1f, 0.5f};
		triangle.transform2d.translation.x = .2f;
		triangle.transform2d.scale = { 2.f, .5f };
		triangle.transform2d.rotation = .25f * glm::two_pi<float>();
		
		std::vector<LveModel::Vertex> squareVertices = {
			{{-0.5f, -0.5f}, {1.0f, 1.0f, 0.0f, 1.0f}},
			{{0.5f, 0.5f}, {1.0f, 1.0f, 0.0f, 1.0f}},
			{{-0.5f, 0.5f}, {1.0f, 1.0f, 0.0f, 1.0f}},
			{{-0.5f, -0.5f}, {1.0f, 1.0f, 0.0f, 1.0f}},
			{{0.5f, -0.5f}, {1.0f, 1.0f, 0.0f, 1.0f}},
			{{0.5f, 0.5f}, {1.0f, 1.0f, 0.0f, 1.0f}}
		};

		auto squareModel = std::make_shared<LveModel>(lveDevice, squareVertices, lveAllocator);

		gameModels.push_back(std::move(squareModel));

		auto square = LveGameObject::createGameObject();
		square.model = gameModels.back();
		square.color = { .1f, .1f, .8f, 0.5f };
		square.transform2d.translation.x = -.2f;
		square.transform2d.scale = { 0.25f, 0.25f };
		square.transform2d.rotation = .25f * glm::two_pi<float>();

		gameObjects.push_back(std::move(square));
		gameObjects.push_back(std::move(triangle));
	}

}