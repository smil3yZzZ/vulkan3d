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
		LveCamera camera{};

		while (!lveWindow.shouldClose()) {
			glfwPollEvents();

			float aspect = lveRenderer.getAspectRatio();
			//camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
			camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 10.f);

			if (auto commandBuffer = lveRenderer.beginFrame()) {
				lveRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
				lveRenderer.endSwapChainRenderPass(commandBuffer);
				lveRenderer.endFrame();
			}
		}
		vkDeviceWaitIdle(lveDevice.device());
	}

	// temporary helper function, creates a 1x1x1 cube centered at offset
	std::unique_ptr<LveModel> createCubeModel(LveDevice& device, glm::vec3 offset, LveAllocator& allocator) {
		std::vector<LveModel::Vertex> vertices{

			// left face (white)
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f, 1.f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f, 1.f}},
			{{-.5f, -.5f, .5f}, {.9f, .9f, .9f, 1.f}},
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f, 1.f}},
			{{-.5f, .5f, -.5f}, {.9f, .9f, .9f, 1.f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f, 1.f}},

			// right face (yellow)
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f, 1.f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f, 1.f}},
			{{.5f, -.5f, .5f}, {.8f, .8f, .1f, 1.f}},
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f, 1.f}},
			{{.5f, .5f, -.5f}, {.8f, .8f, .1f, 1.f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f, 1.f}},

			// top face (orange, remember y axis points down)
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f, 1.f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f, 1.f}},
			{{-.5f, -.5f, .5f}, {.9f, .6f, .1f, 1.f}},
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f, 1.f}},
			{{.5f, -.5f, -.5f}, {.9f, .6f, .1f, 1.f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f, 1.f}},

			// bottom face (red)
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f, 1.f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f, 1.f}},
			{{-.5f, .5f, .5f}, {.8f, .1f, .1f, 1.f}},
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f, 1.f}},
			{{.5f, .5f, -.5f}, {.8f, .1f, .1f, 1.f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f, 1.f}},

			// nose face (blue)
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f, 1.f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f, 1.f}},
			{{-.5f, .5f, 0.5f}, {.1f, .1f, .8f, 1.f}},
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f, 1.f}},
			{{.5f, -.5f, 0.5f}, {.1f, .1f, .8f, 1.f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f, 1.f}},

			// tail face (green)
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f, 1.f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f, 1.f}},
			{{-.5f, .5f, -0.5f}, {.1f, .8f, .1f, 1.f}},
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f, 1.f}},
			{{.5f, -.5f, -0.5f}, {.1f, .8f, .1f, 1.f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f, 1.f}},

		};
		for (auto& v : vertices) {
			v.position += offset;
		}
		return std::make_unique<LveModel>(device, vertices, allocator);
	}

	void FirstApp::loadGameObjects() {
		std::shared_ptr<LveModel> cubeModel = createCubeModel(lveDevice, { .0f, .0f, .0f }, lveAllocator);
		gameModels.push_back(std::move(cubeModel));
		auto cube = LveGameObject::createGameObject();
		cube.model = gameModels.back();
		cube.transform.translation = { .0f, .0f, 2.5f};
		cube.transform.scale = {.5f, .5f, .5f};
		gameObjects.push_back(std::move(cube));
	}

}