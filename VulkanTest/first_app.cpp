#include "first_app.hpp"

#include "keyboard_movement_controller.hpp"
#include "lve_camera.hpp"
#include "simple_render_system.hpp"

#include <math.h>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <chrono>

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
		//camera.setViewTarget(glm::vec3{ -1.f, -2.f, -2.f }, glm::vec3{ 0.f, 0.f, 2.5f });

		auto viewerObject = LveGameObject::createGameObject();

		//viewerObject.transform.translation = glm::vec3{ -1.f, -2.f, -2.f };
		//std::cout << glm::acos(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, glm::normalize(glm::vec3{ 0.f, 2.f, 4.5f }))) << std::endl;
		//viewerObject.transform.rotation.x = - glm::acos(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, glm::normalize(glm::vec3{ 0.f, 2.f, 4.5f })));
		//viewerObject.transform.rotation.y = glm::acos(glm::dot(glm::vec3{ 0.f, 0.f, 1.f }, glm::normalize(glm::vec3{ 1.f, 0.f, 4.5f })));
		KeyboardMovementController cameraController{};

		auto currentTime = std::chrono::high_resolution_clock::now();

		bool buffersCopied = false;

		while (!lveWindow.shouldClose()) {
			glfwPollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			//Limit frameTime to avoid resizing delay
			frameTime = glm::min(frameTime, MIN_SECONDS_PER_FRAME);
			
			cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject);
			camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

			float aspect = lveRenderer.getAspectRatio();
			camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 10.f);

			if (auto commandBuffer = lveRenderer.beginFrame()) {
				if (!buffersCopied) {
					for (auto& v : gameModels) {
						std::cout << "Creating vertex buffer" << std::endl;
						v->createVertexStagingBuffers();
						std::cout << "Copying vertex buffer" << std::endl;
						v->copyVertexStagingBuffers(commandBuffer);
						std::cout << "Creating index buffer" << std::endl;
						v->createIndexStagingBuffers();
						std::cout << "Copying index buffer" << std::endl;
						v->copyIndexStagingBuffers(commandBuffer);
						//create and copy (or create it before) buffers();
					}
					lveTransferer.setCopyBarrier(commandBuffer);
					buffersCopied = true;
				}
				lveRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
				lveRenderer.endSwapChainRenderPass(commandBuffer);
				lveRenderer.endFrame();
			}
		}
		vkDeviceWaitIdle(lveDevice.device());
	}

	// temporary helper function, creates a 1x1x1 cube centered at offset with an index buffer
	std::unique_ptr<LveModel> createCubeModel(LveDevice& device, glm::vec3 offset, LveAllocator& allocator, LveTransferer& transferer) {
		LveModel::Builder modelBuilder{};
		modelBuilder.vertices = {
			// left face (white)
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f, 1.f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f, 1.f}},
			{{-.5f, -.5f, .5f}, {.9f, .9f, .9f, 1.f}},
			{{-.5f, .5f, -.5f}, {.9f, .9f, .9f, 1.f}},

			// right face (yellow)
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f, 1.f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f, 1.f}},
			{{.5f, -.5f, .5f}, {.8f, .8f, .1f, 1.f}},
			{{.5f, .5f, -.5f}, {.8f, .8f, .1f, 1.f}},

			// top face (orange, remember y axis points down)
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f, 1.f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f, 1.f}},
			{{-.5f, -.5f, .5f}, {.9f, .6f, .1f, 1.f}},
			{{.5f, -.5f, -.5f}, {.9f, .6f, .1f, 1.f}},

			// bottom face (red)
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f, 1.f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f, 1.f}},
			{{-.5f, .5f, .5f}, {.8f, .1f, .1f, 1.f}},
			{{.5f, .5f, -.5f}, {.8f, .1f, .1f, 1.f}},

			// nose face (blue)
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f, 1.f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f, 1.f}},
			{{-.5f, .5f, 0.5f}, {.1f, .1f, .8f, 1.f}},
			{{.5f, -.5f, 0.5f}, {.1f, .1f, .8f, 1.f}},

			// tail face (green)
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f, 1.f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f, 1.f}},
			{{-.5f, .5f, -0.5f}, {.1f, .8f, .1f, 1.f}},
			{{.5f, -.5f, -0.5f}, {.1f, .8f, .1f, 1.f}},
		};
		for (auto& v : modelBuilder.vertices) {
			v.position += offset;
		}

		modelBuilder.indices = { 0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
								12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21 };

		return std::make_unique<LveModel>(device, modelBuilder, allocator, transferer);
	}

	void FirstApp::loadGameObjects() {
		std::shared_ptr<LveModel> cubeModel = createCubeModel(lveDevice, { .0f, .0f, .0f }, lveAllocator, lveTransferer);
		gameModels.push_back(std::move(cubeModel));
		auto cube = LveGameObject::createGameObject();
		cube.model = gameModels.back();
		cube.transform.translation = { .0f, .0f, 2.5f};
		cube.transform.scale = {.5f, .5f, .5f};
		gameObjects.push_back(std::move(cube));
	}

}