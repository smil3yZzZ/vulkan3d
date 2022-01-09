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
				lveRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
				lveRenderer.endSwapChainRenderPass(commandBuffer);
				lveRenderer.endFrame();
			}
		}
		vkDeviceWaitIdle(lveDevice.device());
	}

	void FirstApp::loadGameObjects() {
		std::shared_ptr<LveModel> flatVaseModel = LveModel::createModelFromFile(lveDevice, "models/flat_vase.obj", lveAllocator);
		gameModels.push_back(std::move(flatVaseModel));
		auto flatVase = LveGameObject::createGameObject();
		flatVase.model = gameModels.back();
		flatVase.transform.translation = { -.5f, .5f, 2.5f};
		flatVase.transform.scale = glm::vec3(3.f, 1.5f, 3.f);
		gameObjects.push_back(std::move(flatVase));

		std::shared_ptr<LveModel> smoothVaseModel = LveModel::createModelFromFile(lveDevice, "models/smooth_vase.obj", lveAllocator);
		gameModels.push_back(std::move(smoothVaseModel));
		auto smoothVase = LveGameObject::createGameObject();
		smoothVase.model = gameModels.back();
		smoothVase.transform.translation = { .5f, .5f, 2.5f };
		smoothVase.transform.scale = glm::vec3(3.f, 1.5f, 3.f);
		gameObjects.push_back(std::move(smoothVase));
	}

}