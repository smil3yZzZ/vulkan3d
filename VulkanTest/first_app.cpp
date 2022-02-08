#include "first_app.hpp"

#include "keyboard_movement_controller.hpp"
#include "lve_camera.hpp"
#include "systems/simple_render_system.hpp"
#include "systems/point_light_system.hpp"

#include <math.h>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <numeric>
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
		SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), lveRenderer.getGBufferDescriptorSetLayout(), lveRenderer.getCompositionDescriptorSetLayout() };
		PointLightSystem pointLightSystem{ lveDevice, lveRenderer.getSwapChainRenderPass(), lveRenderer.getGBufferDescriptorSetLayout(), lveRenderer.getCompositionDescriptorSetLayout() };
		LveCamera camera{};

		auto viewerObject = LveGameObject::createGameObject();
		viewerObject.transform.translation.z = -2.5f;

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
				int frameIndex = lveRenderer.getFrameIndex();

				FrameInfo frameInfo{
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					lveRenderer.getCurrentGBufferDescriptorSet(),
					lveRenderer.getCurrentCompositionDescriptorSet(),
					gameObjects
				};
				// update
				LveSwapChain::GBufferUbo gBufferUbo{};
				gBufferUbo.projection = camera.getProjection();
				gBufferUbo.view = camera.getView();
				
				/*
				lveRenderer.getCurrentGBufferUbo().writeToBuffer(&gBufferUbo);
				lveRenderer.getCurrentGBufferUbo().flush();
				*/
				lveRenderer.updateCurrentGBufferUbo(&gBufferUbo);

				LveSwapChain::CompositionUbo compositionUbo{};
				compositionUbo.viewPos = viewerObject.transform.translation;

				lveRenderer.updateCurrentCompositionUbo(&compositionUbo);
				/*
				lveRenderer.getCurrentCompositionUbo().writeToBuffer(&compositionUbo);
				lveRenderer.getCurrentCompositionUbo().flush();
				*/

				// render
				lveRenderer.beginSwapChainRenderPass(commandBuffer);
				VkExtent2D extent = lveRenderer.getExtent();
				simpleRenderSystem.renderGameObjects(frameInfo, glm::inverse(camera.getProjection() * camera.getView()), glm::vec2(1.f/extent.width, 1.f/extent.height));
				pointLightSystem.render(frameInfo);
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
		flatVase.transform.translation = { -.5f, .5f, 0.f};
		flatVase.transform.scale = glm::vec3(3.f, 1.5f, 3.f);
		gameObjects.emplace(flatVase.getId(), std::move(flatVase));

		std::shared_ptr<LveModel> smoothVaseModel = LveModel::createModelFromFile(lveDevice, "models/smooth_vase.obj", lveAllocator);
		gameModels.push_back(std::move(smoothVaseModel));
		auto smoothVase = LveGameObject::createGameObject();
		smoothVase.model = gameModels.back();
		smoothVase.transform.translation = { .5f, .5f, 0.f };
		smoothVase.transform.scale = glm::vec3(3.f, 1.5f, 3.f);
		gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

		std::shared_ptr<LveModel> quadModel = LveModel::createModelFromFile(lveDevice, "models/quad.obj", lveAllocator);
		gameModels.push_back(std::move(quadModel));
		auto floor = LveGameObject::createGameObject();
		floor.model = gameModels.back();
		floor.transform.translation = {0.f, .5f, 0.f };
		floor.transform.scale = glm::vec3(3.f, 1.f, 3.f);
		gameObjects.emplace(floor.getId(), std::move(floor));

		std::shared_ptr<LveModel> coloredCubeModel = LveModel::createModelFromFile(lveDevice, "models/colored_cube.obj", lveAllocator);
		gameModels.push_back(std::move(coloredCubeModel));
		auto coloredCube = LveGameObject::createGameObject();
		coloredCube.model = gameModels.back();
		coloredCube.transform.translation = { 0.f, 0.f, 1.f };
		coloredCube.transform.scale = glm::vec3(0.5f, 0.5f, 0.5f);
		gameObjects.emplace(coloredCube.getId(), std::move(coloredCube));


	}

}