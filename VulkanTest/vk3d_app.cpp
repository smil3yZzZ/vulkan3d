#include "vk3d_app.hpp"

#include "keyboard_movement_controller.hpp"
#include "vk3d_camera.hpp"
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


namespace vk3d {
	Vk3dApp::Vk3dApp() {
		loadGameObjects();
	}

	Vk3dApp::~Vk3dApp() {
	}

	void Vk3dApp::run() {
		ShadowRenderSystem shadowRenderSystem{ lveDevice, lveRenderer.getShadowRenderPass() };
		SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), lveRenderer.getGBufferDescriptorSetLayout(), lveRenderer.getCompositionDescriptorSetLayout() };
		PointLightSystem pointLightSystem{ lveDevice, lveRenderer.getSwapChainRenderPass(), lveRenderer.getGBufferDescriptorSetLayout(), lveRenderer.getCompositionDescriptorSetLayout() };
		Vk3dCamera camera{};
		Vk3dCamera light{};

		auto viewerObject = Vk3dGameObject::createGameObject();

		auto lightObject = Vk3dGameObject::createGameObject();

		KeyboardMovementController cameraController{};

		auto currentTime = std::chrono::high_resolution_clock::now();

		// update
		Vk3dSwapChain::GBufferUbo gBufferUbo{};
		Vk3dSwapChain::CompositionUbo compositionUbo{};

		viewerObject.transform.translation = compositionUbo.lightPosition;
		viewerObject.transform.rotation.x = -.9f;
		viewerObject.transform.rotation.y = 0.75f;
		
		lightObject.transform.translation = compositionUbo.lightPosition;
		lightObject.transform.rotation.x = -.9f;
		lightObject.transform.rotation.y = 0.75f;

		light.setViewYXZ(lightObject.transform.translation, lightObject.transform.rotation);
		float aspect = lveRenderer.getShadowAspectRatio();
		light.setPerspectiveProjection(glm::radians(60.0f), aspect, 0.1f, 24.0f);

		glm::mat4 lightProjView = light.getProjection() * light.getView();

		gBufferUbo.lightProjectionView = lightProjView;

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
			camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 50.0f);

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
				
				gBufferUbo.projection = camera.getProjection();
				gBufferUbo.view = camera.getView();

				lveRenderer.updateCurrentGBufferUbo(&gBufferUbo);

				compositionUbo.viewPos = viewerObject.transform.translation;

				lveRenderer.updateCurrentCompositionUbo(&compositionUbo);

				// render shadows
				lveRenderer.beginShadowRenderPass(commandBuffer);
				shadowRenderSystem.renderGameObjects(frameInfo, lightProjView);
				lveRenderer.endShadowRenderPass(commandBuffer);

				// render swap chain
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

	void Vk3dApp::loadGameObjects() {

		std::shared_ptr<Vk3dModel> quadModel = Vk3dModel::createModelFromFile(lveDevice, "models/quad.obj", lveAllocator);
		gameModels.push_back(std::move(quadModel));
		auto floor = Vk3dGameObject::createGameObject();
		floor.model = gameModels.back();
		floor.transform.translation = {0.f, 0.f, 0.f };
		floor.transform.scale = glm::vec3(12.f, 1.f, 12.f);
		gameObjects.emplace(floor.getId(), std::move(floor));

		auto right = Vk3dGameObject::createGameObject();
		right.model = gameModels.back();
		right.transform.translation = { 12.f, -12.f, 0.f };
		right.transform.scale = glm::vec3(12.f, 1.f, 12.f);
		right.transform.rotation = glm::vec3(0.f, 0.f, glm::radians(90.0f));
		gameObjects.emplace(right.getId(), std::move(right));

		auto front = Vk3dGameObject::createGameObject();
		front.model = gameModels.back();
		front.transform.translation = { 0.f, -12.f, 12.f };
		front.transform.scale = glm::vec3(12.f, 1.f, 12.f);
		front.transform.rotation = glm::vec3(glm::radians(90.0f), 0.f, 0.f);
		gameObjects.emplace(front.getId(), std::move(front));

		auto left = Vk3dGameObject::createGameObject();
		left.model = gameModels.back();
		left.transform.translation = { -12.f, -12.f, 0.f };
		left.transform.scale = glm::vec3(12.f, 1.f, 12.f);
		left.transform.rotation = glm::vec3(0.f, 0.f, glm::radians(90.0f));
		gameObjects.emplace(left.getId(), std::move(left));

		auto back = Vk3dGameObject::createGameObject();
		back.model = gameModels.back();
		back.transform.translation = { 0.f, -12.f, -12.f };
		back.transform.scale = glm::vec3(12.f, 1.f, 12.f);
		back.transform.rotation = glm::vec3(glm::radians(90.0f), 0.f, 0.f);
		gameObjects.emplace(back.getId(), std::move(back));

		auto top = Vk3dGameObject::createGameObject();
		top.model = gameModels.back();
		top.transform.translation = { 0.f, -12.f, 0.f };
		top.transform.scale = glm::vec3(12.f, 1.f, 12.f);
		gameObjects.emplace(top.getId(), std::move(top));

		std::shared_ptr<Vk3dModel> coloredCubeModel = Vk3dModel::createModelFromFile(lveDevice, "models/colored_cube.obj", lveAllocator);
		gameModels.push_back(std::move(coloredCubeModel));
		auto coloredCube = Vk3dGameObject::createGameObject();
		coloredCube.model = gameModels.back();
		coloredCube.transform.translation = { 1.f, -1.f, 1.f };
		coloredCube.transform.scale = glm::vec3(0.5f, 1.f, 0.5f);
		gameObjects.emplace(coloredCube.getId(), std::move(coloredCube));

		auto coloredCube2 = Vk3dGameObject::createGameObject();
		coloredCube2.model = gameModels.back();
		coloredCube2.transform.translation = { -.5f, -1.f, 2.f };
		coloredCube2.transform.scale = glm::vec3(0.5f, 1.f, 0.5f);
		gameObjects.emplace(coloredCube2.getId(), std::move(coloredCube2));

		auto coloredCube3 = Vk3dGameObject::createGameObject();
		coloredCube3.model = gameModels.back();
		coloredCube3.transform.translation = { .5f, -1.f, 3.f };
		coloredCube3.transform.scale = glm::vec3(0.5f, 1.f, 0.5f);
		gameObjects.emplace(coloredCube3.getId(), std::move(coloredCube3));


	}

}