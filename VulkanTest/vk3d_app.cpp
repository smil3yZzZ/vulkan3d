#include "vk3d_app.hpp"

#include "keyboard_movement_controller.hpp"
#include "vk3d_camera.hpp"
#include "systems/shadow_render_system.hpp"
#include "systems/scene_render_system.hpp"
#include "systems/reflection_render_system.hpp"
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
		ShadowRenderSystem shadowRenderSystem{ vk3dDevice, vk3dRenderer.getShadowRenderPass(), vk3dRenderer.getShadowDescriptorSetLayout() };
		ReflectionRenderSystem reflectionRenderSystem{ vk3dDevice, vk3dRenderer.getMappingsRenderPass(), vk3dRenderer.getMappingsDescriptorSetLayout(), vk3dRenderer.getUVReflectionRenderPass(), vk3dRenderer.getUVReflectionDescriptorSetLayout() };
		SceneRenderSystem sceneRenderSystem{
			vk3dDevice, 
			vk3dRenderer.getLightingRenderPass(), 
			vk3dRenderer.getGBufferDescriptorSetLayout(), 
			vk3dRenderer.getCompositionDescriptorSetLayout(),
			vk3dRenderer.getPostProcessingRenderPass(),
			vk3dRenderer.getPostProcessingDescriptorSetLayout()};
		PointLightSystem pointLightSystem{ vk3dDevice, vk3dRenderer.getLightingRenderPass(), vk3dRenderer.getGBufferDescriptorSetLayout(), vk3dRenderer.getCompositionDescriptorSetLayout() };
		Vk3dCamera camera{};
		Vk3dCamera light{};

		auto viewerObject = Vk3dGameObject::createGameObject();

		auto lightObject = Vk3dGameObject::createGameObject();

		KeyboardMovementController cameraController{};

		auto currentTime = std::chrono::high_resolution_clock::now();

		Vk3dSwapChain::ShadowUbo shadowUbo{};
		Vk3dSwapChain::GBufferUbo gBufferUbo{};
		Vk3dSwapChain::CompositionUbo compositionUbo{};
		Vk3dSwapChain::MappingsUbo mappingsUbo{};
		Vk3dSwapChain::UVReflectionUbo uvReflectionUbo{};
		Vk3dSwapChain::PostProcessingUbo postProcessingUbo{};

		viewerObject.transform.translation = Vk3dSwapChain::CAMERA_POSITION;
		
		lightObject.transform.translation = Vk3dSwapChain::LIGHT_POSITION;

		float aspect = vk3dRenderer.getShadowAspectRatio();
		light.setPerspectiveProjection(glm::radians(90.0f), aspect, LIGHT_NEAR_PLANE, LIGHT_FAR_PLANE);

		for (int faceIndex = 0; faceIndex < Vk3dSwapChain::NUM_CUBE_FACES; faceIndex++) {
			
			lightObject.transform.resetRotation();

			switch (faceIndex)
			{
			case 0: // POSITIVE_X
				lightObject.transform.rotation.y = glm::radians(90.0f);
				break;
			case 1:	// NEGATIVE_X
				lightObject.transform.rotation.y = glm::radians(-90.0f);
				break;
			case 2:	// POSITIVE_Y
				lightObject.transform.rotation.x = glm::radians(90.0f);
				break;
			case 3:	// NEGATIVE_Y
				lightObject.transform.rotation.x = glm::radians(-90.0f);
				break;
			case 4:	// POSITIVE_Z
				break;
			case 5:	// NEGATIVE_Z
				lightObject.transform.rotation.y = glm::radians(180.0f);
				break;
			}
			light.setViewYXZ(lightObject.transform.translation, lightObject.transform.rotation);
			shadowUbo.projectionView[faceIndex] = light.getProjection() * light.getView();
		}

		VkExtent2D extent = vk3dRenderer.getExtent();
		glm::vec2 invResolution = glm::vec2(1.f / extent.width, 1.f / extent.height);
		uvReflectionUbo.invResolution = invResolution;
		postProcessingUbo.invResolution = invResolution;

		while (!vk3dWindow.shouldClose()) {
			glfwPollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			//Limit frameTime to avoid resizing delay
			frameTime = glm::min(frameTime, MIN_SECONDS_PER_FRAME);
			
			cameraController.moveInPlaneXZ(vk3dWindow.getGLFWwindow(), frameTime, viewerObject);
			camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

			float aspect = vk3dRenderer.getAspectRatio();
			camera.setPerspectiveProjection(glm::radians(50.0f), aspect, CAMERA_NEAR_PLANE, CAMERA_FAR_PLANE);

			if (auto commandBuffer = vk3dRenderer.beginFrame()) {
				int frameIndex = vk3dRenderer.getFrameIndex();

				FrameInfo frameInfo{
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					vk3dRenderer.getCurrentShadowDescriptorSet(),
					vk3dRenderer.getCurrentMappingsDescriptorSet(),
					vk3dRenderer.getCurrentUVReflectionDescriptorSet(),
					vk3dRenderer.getCurrentGBufferDescriptorSet(),
					vk3dRenderer.getCurrentCompositionDescriptorSet(),
					vk3dRenderer.getCurrentPostProcessingDescriptorSet(),
					gameObjects
				};

				vk3dRenderer.updateCurrentShadowUbo(&shadowUbo);
				
				gBufferUbo.projection = camera.getProjection();
				gBufferUbo.view = camera.getView();

				vk3dRenderer.updateCurrentGBufferUbo(&gBufferUbo);

				mappingsUbo.projection = camera.getProjection();
				mappingsUbo.view = camera.getView();

				vk3dRenderer.updateCurrentMappingsUbo(&mappingsUbo);

				uvReflectionUbo.viewPos = viewerObject.transform.translation;
				uvReflectionUbo.projection = camera.getProjection();
				uvReflectionUbo.view = camera.getView();

				vk3dRenderer.updateCurrentUVReflectionUbo(&uvReflectionUbo);

				compositionUbo.viewPos = viewerObject.transform.translation;

				vk3dRenderer.updateCurrentCompositionUbo(&compositionUbo);

				vk3dRenderer.updateCurrentPostProcessingUbo(&postProcessingUbo);

				// render shadows
				vk3dRenderer.beginShadowRenderPass(commandBuffer);
				shadowRenderSystem.renderGameObjects(frameInfo);
				vk3dRenderer.endRenderPass(commandBuffer);

				// render mappings
				vk3dRenderer.beginMappingsRenderPass(commandBuffer);
				reflectionRenderSystem.renderMappings(frameInfo);
				vk3dRenderer.endRenderPass(commandBuffer);

				// render reflection map
				vk3dRenderer.beginUVReflectionRenderPass(commandBuffer);
				reflectionRenderSystem.renderUVReflectionMap(frameInfo);
				vk3dRenderer.endRenderPass(commandBuffer);

				// render swap chain
				vk3dRenderer.beginLightingRenderPass(commandBuffer);
				sceneRenderSystem.renderGameObjectsLighting(frameInfo, glm::inverse(camera.getProjection() * camera.getView()), invResolution);
				pointLightSystem.render(frameInfo);
				vk3dRenderer.endRenderPass(commandBuffer);
				vk3dRenderer.beginPostProcessingRenderPass(commandBuffer);
				sceneRenderSystem.renderPostProcessing(frameInfo);
				vk3dRenderer.endRenderPass(commandBuffer);
				vk3dRenderer.endFrame();
			}
		}
		vkDeviceWaitIdle(vk3dDevice.device());
	}

	void Vk3dApp::loadGameObjects() {

		std::shared_ptr<Vk3dModel> quadModel = Vk3dModel::createModelFromFile(vk3dDevice, "models/quad.obj", vk3dAllocator);

		gameModels.push_back(std::move(quadModel));
		auto floorFar = Vk3dGameObject::createGameObject();
		floorFar.model = gameModels.back();
		floorFar.transform.translation = {0.f, 0.f, 6.f };
		floorFar.transform.scale = glm::vec3(9.f, 1.f, 3.f);
		gameObjects.emplace(floorFar.getId(), std::move(floorFar));

		auto floorLeft = Vk3dGameObject::createGameObject();
		floorLeft.model = gameModels.back();
		floorLeft.transform.translation = { -6.f, 0.f, 0.f };
		floorLeft.transform.scale = glm::vec3(3.f, 1.f, 6.f);
		gameObjects.emplace(floorLeft.getId(), std::move(floorLeft));

		auto floorRight = Vk3dGameObject::createGameObject();
		floorRight.model = gameModels.back();
		floorRight.transform.translation = { 6.f, 0.f, 0.f };
		floorRight.transform.scale = glm::vec3(3.f, 1.f, 6.f);
		gameObjects.emplace(floorRight.getId(), std::move(floorRight));

		auto right = Vk3dGameObject::createGameObject();
		right.model = gameModels.back();
		right.transform.translation = { 9.f, -9.f, 0.f };
		right.transform.scale = glm::vec3(9.f, 1.f, 9.f);
		right.transform.rotation = glm::vec3(0.f, 0.f, glm::radians(-90.0f));
		gameObjects.emplace(right.getId(), std::move(right));

		auto front = Vk3dGameObject::createGameObject();
		front.model = gameModels.back();
		front.transform.translation = { 0.f, -9.f, 9.f };
		front.transform.scale = glm::vec3(9.f, 1.f, 9.f);
		front.transform.rotation = glm::vec3(glm::radians(90.0f), 0.f, 0.f);
		gameObjects.emplace(front.getId(), std::move(front));

		auto left = Vk3dGameObject::createGameObject();
		left.model = gameModels.back();
		left.transform.translation = { -9.f, -9.f, 0.f };
		left.transform.scale = glm::vec3(9.f, 1.f, 9.f);
		left.transform.rotation = glm::vec3(0.f, 0.f, glm::radians(90.0f));
		gameObjects.emplace(left.getId(), std::move(left));

		auto back = Vk3dGameObject::createGameObject();
		back.model = gameModels.back();
		back.transform.translation = { 0.f, -9.f, -9.f };
		back.transform.scale = glm::vec3(9.f, 1.f, 9.f);
		back.transform.rotation = glm::vec3(glm::radians(-90.0f), 0.f, 0.f);
		gameObjects.emplace(back.getId(), std::move(back));

		auto top = Vk3dGameObject::createGameObject();
		top.model = gameModels.back();
		top.transform.translation = { 0.f, -18.f, 0.f };
		top.transform.scale = glm::vec3(9.f, 1.f, 9.f);
		top.transform.rotation = glm::vec3(glm::radians(180.0f), 0.f, 0.f);
		gameObjects.emplace(top.getId(), std::move(top));

		std::shared_ptr<Vk3dModel> mirrorQuadModel = Vk3dModel::createModelFromFile(vk3dDevice, "models/mirror_quad.obj", vk3dAllocator);

		gameModels.push_back(std::move(mirrorQuadModel));
		auto floorMirror = Vk3dGameObject::createGameObject();
		floorMirror.model = gameModels.back();
		floorMirror.transform.translation = { 0.f, 0.f, 0.f };
		floorMirror.transform.scale = glm::vec3(3.f, 1.f, 3.f);
		floorMirror.reflection = 1.0f;
		gameObjects.emplace(floorMirror.getId(), std::move(floorMirror));
		

		std::shared_ptr<Vk3dModel> coloredCubeModel = Vk3dModel::createModelFromFile(vk3dDevice, "models/colored_cube.obj", vk3dAllocator);
		gameModels.push_back(std::move(coloredCubeModel));
		auto coloredCube = Vk3dGameObject::createGameObject();
		coloredCube.model = gameModels.back();
		coloredCube.transform.translation = { 1.f, -1.f, 0.5f };
		coloredCube.transform.scale = glm::vec3(0.5f, 1.f, 0.5f);
		gameObjects.emplace(coloredCube.getId(), std::move(coloredCube));

		auto coloredCube2 = Vk3dGameObject::createGameObject();
		coloredCube2.model = gameModels.back();
		coloredCube2.transform.translation = { -.5f, -1.f, 1.5f };
		coloredCube2.transform.scale = glm::vec3(0.5f, 1.f, 0.5f);
		gameObjects.emplace(coloredCube2.getId(), std::move(coloredCube2));

		auto coloredCube3 = Vk3dGameObject::createGameObject();
		coloredCube3.model = gameModels.back();
		coloredCube3.transform.translation = { .5f, -1.f, 2.5f };
		coloredCube3.transform.scale = glm::vec3(0.5f, 1.f, 0.5f);
		gameObjects.emplace(coloredCube3.getId(), std::move(coloredCube3));
	}

}