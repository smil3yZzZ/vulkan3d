#include "first_app.hpp"

#include "keyboard_movement_controller.hpp"
#include "lve_camera.hpp"
#include "systems/shadow_render_system.hpp"
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
		ShadowRenderSystem shadowRenderSystem{ lveDevice, lveRenderer.getShadowRenderPass(),  lveRenderer.getShadowDescriptorSetLayout() };
		SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), lveRenderer.getGBufferDescriptorSetLayout(), lveRenderer.getCompositionDescriptorSetLayout() };
		PointLightSystem pointLightSystem{ lveDevice, lveRenderer.getSwapChainRenderPass(), lveRenderer.getGBufferDescriptorSetLayout(), lveRenderer.getCompositionDescriptorSetLayout() };
		LveCamera camera{};
		LveCamera light{};

		auto viewerObject = LveGameObject::createGameObject();

		auto lightObject = LveGameObject::createGameObject();

		KeyboardMovementController cameraController{};

		auto currentTime = std::chrono::high_resolution_clock::now();

		// update
		LveSwapChain::GBufferUbo gBufferUbo{};
		LveSwapChain::CompositionUbo compositionUbo{};
		LveSwapChain::ShadowUbo shadowUbo{};

		viewerObject.transform.translation = compositionUbo.lightPosition;
		viewerObject.transform.rotation.x = -.9f;
		viewerObject.transform.rotation.y = 0.75f;
		
		lightObject.transform.translation = compositionUbo.lightPosition;
		lightObject.transform.rotation.x = -.9f;
		lightObject.transform.rotation.y = 0.75f;

		light.setViewYXZ(lightObject.transform.translation, lightObject.transform.rotation);
		float aspect = lveRenderer.getShadowAspectRatio();
		light.setPerspectiveProjection(glm::radians(45.0f), aspect, LIGHT_NEAR, LIGHT_FAR);

		glm::mat4 lightProjView = light.getProjection() * light.getView();

		//gBufferUbo.lightProjectionView = lightProjView;
		shadowUbo.lightFarPlane = LIGHT_FAR;

		while (!lveWindow.shouldClose()) {
			glfwPollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			std::cout << frameTime << std::endl;

			//Limit frameTime to avoid resizing delay
			frameTime = glm::min(frameTime, MIN_SECONDS_PER_FRAME);
			
			cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject);
			camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

			float aspect = lveRenderer.getAspectRatio();
			camera.setPerspectiveProjection(glm::radians(50.0f), aspect, CAMERA_NEAR, CAMERA_FAR);

			if (auto commandBuffer = lveRenderer.beginFrame()) {

				int frameIndex = lveRenderer.getFrameIndex();

				FrameInfo frameInfo{
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					lveRenderer.getCurrentShadowDescriptorSet(),
					lveRenderer.getCurrentGBufferDescriptorSet(),
					lveRenderer.getCurrentCompositionDescriptorSet(),
					gameObjects
				};
				
				gBufferUbo.projection = camera.getProjection();
				gBufferUbo.view = camera.getView();

				lveRenderer.updateCurrentGBufferUbo(&gBufferUbo);

				compositionUbo.viewPos = viewerObject.transform.translation;

				lveRenderer.updateCurrentCompositionUbo(&compositionUbo);
				lveRenderer.updateCurrentShadowUbo(&shadowUbo);

				// check multithreading here
				// render shadows
				//lveRenderer.beginShadowRenderPassConfig(commandBuffer);
				// use a cube to have omnidirectional shadows
				std::vector<VkCommandBuffer> parallelCommandBuffers = shadowRenderSystem.executeRenderPassCommands(frameInfo, light.getProjection(), light.getView(), &lveRenderer);

				// render swap chain
				lveRenderer.beginSwapChainRenderPass(commandBuffer);
				VkExtent2D extent = lveRenderer.getExtent();
				simpleRenderSystem.renderGameObjects(frameInfo, glm::inverse(camera.getProjection() * camera.getView()), glm::vec2(1.f/extent.width, 1.f/extent.height));
				pointLightSystem.render(frameInfo);
				lveRenderer.endSwapChainRenderPass(commandBuffer);
				lveRenderer.endFrame(parallelCommandBuffers);
			}
		}
		vkDeviceWaitIdle(lveDevice.device());
	}

	void FirstApp::loadGameObjects() {

		std::shared_ptr<LveModel> quadModel = LveModel::createModelFromFile(lveDevice, "models/quad.obj", lveAllocator);
		gameModels.push_back(std::move(quadModel));
		auto floor = LveGameObject::createGameObject();
		floor.model = gameModels.back();
		floor.transform.translation = {0.f, 0.f, 0.f };
		floor.transform.scale = glm::vec3(12.f, 1.f, 12.f);
		gameObjects.emplace(floor.getId(), std::move(floor));

		auto right = LveGameObject::createGameObject();
		right.model = gameModels.back();
		right.transform.translation = { 12.f, -12.f, 0.f };
		right.transform.scale = glm::vec3(12.f, 1.f, 12.f);
		right.transform.rotation = glm::vec3(0.f, 0.f, glm::radians(90.0f));
		gameObjects.emplace(right.getId(), std::move(right));

		auto front = LveGameObject::createGameObject();
		front.model = gameModels.back();
		front.transform.translation = { 0.f, -12.f, 12.f };
		front.transform.scale = glm::vec3(12.f, 1.f, 12.f);
		front.transform.rotation = glm::vec3(glm::radians(90.0f), 0.f, 0.f);
		gameObjects.emplace(front.getId(), std::move(front));

		auto left = LveGameObject::createGameObject();
		left.model = gameModels.back();
		left.transform.translation = { -12.f, -12.f, 0.f };
		left.transform.scale = glm::vec3(12.f, 1.f, 12.f);
		left.transform.rotation = glm::vec3(0.f, 0.f, glm::radians(90.0f));
		gameObjects.emplace(left.getId(), std::move(left));

		auto back = LveGameObject::createGameObject();
		back.model = gameModels.back();
		back.transform.translation = { 0.f, -12.f, -12.f };
		back.transform.scale = glm::vec3(12.f, 1.f, 12.f);
		back.transform.rotation = glm::vec3(glm::radians(90.0f), 0.f, 0.f);
		gameObjects.emplace(back.getId(), std::move(back));

		auto top = LveGameObject::createGameObject();
		top.model = gameModels.back();
		top.transform.translation = { 0.f, -12.f, 0.f };
		top.transform.scale = glm::vec3(12.f, 1.f, 12.f);
		gameObjects.emplace(top.getId(), std::move(top));

		std::shared_ptr<LveModel> coloredCubeModel = LveModel::createModelFromFile(lveDevice, "models/colored_cube.obj", lveAllocator);
		gameModels.push_back(std::move(coloredCubeModel));
		auto coloredCube = LveGameObject::createGameObject();
		coloredCube.model = gameModels.back();
		coloredCube.transform.translation = { 1.f, -1.f, 1.f };
		coloredCube.transform.scale = glm::vec3(0.5f, 1.f, 0.5f);
		gameObjects.emplace(coloredCube.getId(), std::move(coloredCube));

		auto coloredCube2 = LveGameObject::createGameObject();
		coloredCube2.model = gameModels.back();
		coloredCube2.transform.translation = { -.5f, -1.f, 2.f };
		coloredCube2.transform.scale = glm::vec3(0.5f, 1.f, 0.5f);
		gameObjects.emplace(coloredCube2.getId(), std::move(coloredCube2));

		auto coloredCube3 = LveGameObject::createGameObject();
		coloredCube3.model = gameModels.back();
		coloredCube3.transform.translation = { .5f, -1.f, 3.f };
		coloredCube3.transform.scale = glm::vec3(0.5f, 1.f, 0.5f);
		gameObjects.emplace(coloredCube3.getId(), std::move(coloredCube3));

	}

}