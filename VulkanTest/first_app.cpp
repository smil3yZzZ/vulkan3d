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

	/*
	struct GlobalUbo {
		glm::vec3 viewPos;
		alignas(16) glm::mat4 projection{ 1.f };
		glm::mat4 view{ 1.f };
		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .05f }; //w is intensity
		glm::vec3 lightPosition{ -1.f };
		alignas(16) glm::vec4 lightColor{ .8f, 1.f, .2f, 1.f }; // w is light intensity
	};
	*/

	struct GBufferUbo {
		glm::mat4 projection{ 1.f };
		glm::mat4 view{ 1.f };
	};

	struct CompositionUbo {
		glm::vec3 viewPos;
		alignas(16) glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .05f }; //w is intensity
		glm::vec3 lightPosition{ -1.f };
		alignas(16) glm::vec4 lightColor{ .8f, 1.f, .2f, 1.f }; // w is light intensity
	};

	FirstApp::FirstApp() {
		globalPool = LveDescriptorPool::Builder(lveDevice)
			.setMaxSets(2 * LveSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2 * LveSwapChain::MAX_FRAMES_IN_FLIGHT) 
			.addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 4 * LveSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();
		loadGameObjects();
	}

	FirstApp::~FirstApp() {
	}

	void FirstApp::run() {
		std::vector<std::unique_ptr<LveBuffer>> gBufferUboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
		std::vector<std::unique_ptr<LveBuffer>> compositionUboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < LveSwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
			gBufferUboBuffers[i] = std::make_unique<LveBuffer>(
				lveDevice,
				sizeof(GBufferUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VMA_MEMORY_USAGE_CPU_TO_GPU,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
				lveAllocator
			);
			gBufferUboBuffers[i]->map();
			compositionUboBuffers[i] = std::make_unique<LveBuffer>(
				lveDevice,
				sizeof(CompositionUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VMA_MEMORY_USAGE_CPU_TO_GPU,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
				lveAllocator
				);
			compositionUboBuffers[i]->map();
		}

		auto gBufferSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.build();

		std::vector<VkDescriptorSet> gBufferDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < gBufferDescriptorSets.size(); i++) {
			auto bufferInfo = gBufferUboBuffers[i]->descriptorInfo();
			LveDescriptorWriter(*gBufferSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(gBufferDescriptorSets[i]);
		}

		auto compositionSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(2, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(3, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		std::vector<VkDescriptorSet> compositionDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < compositionDescriptorSets.size(); i++) {
			auto bufferInfo = compositionUboBuffers[i]->descriptorInfo();
			auto positionInfo = lveRenderer.getSwapChainAttachments()->position.descriptorInfo();
			auto normalInfo = lveRenderer.getSwapChainAttachments()->normal.descriptorInfo();
			auto albedoInfo = lveRenderer.getSwapChainAttachments()->albedo.descriptorInfo();
			auto depthInfo = lveRenderer.getSwapChainAttachments()->depth.descriptorInfo();
			LveDescriptorWriter(*compositionSetLayout, *globalPool)
				.writeImage(0, &positionInfo)
				.writeImage(1, &normalInfo)
				.writeImage(2, &albedoInfo)
				.writeImage(3, &depthInfo)
				.writeBuffer(4, &bufferInfo)
				/*
				.writeImage(0, &)
				.writeImage(1, &bufferInfo)
				.writeImage(2, &bufferInfo)
				.writeImage(3, &bufferInfo)
				.writeBuffer(4, &bufferInfo)
				*/
				.build(compositionDescriptorSets[i]);
		}

		SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), gBufferSetLayout->getDescriptorSetLayout(), compositionSetLayout->getDescriptorSetLayout()};
		//PointLightSystem pointLightSystem{ lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
		LveCamera camera{};
		//camera.setViewTarget(glm::vec3{ -1.f, -2.f, -2.f }, glm::vec3{ 0.f, 0.f, 2.5f });

		auto viewerObject = LveGameObject::createGameObject();
		viewerObject.transform.translation.z = -2.5f;

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
				int frameIndex = lveRenderer.getFrameIndex();
				FrameInfo frameInfo{
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					gBufferDescriptorSets[frameIndex],
					compositionDescriptorSets[frameIndex],
					gameObjects
				};
				// update
				GBufferUbo gBufferUbo{};
				gBufferUbo.projection = camera.getProjection();
				gBufferUbo.view = camera.getView();
				
				gBufferUboBuffers[frameIndex]->writeToBuffer(&gBufferUbo);
				gBufferUboBuffers[frameIndex]->flush();

				CompositionUbo compositionUbo{};
				compositionUbo.viewPos = viewerObject.transform.translation;

				compositionUboBuffers[frameIndex]->writeToBuffer(&compositionUbo);
				compositionUboBuffers[frameIndex]->flush();

				// render
				lveRenderer.beginSwapChainRenderPass(commandBuffer);
				VkExtent2D extent = lveRenderer.getExtent();
				simpleRenderSystem.renderGameObjects(frameInfo, glm::inverse(camera.getProjection() * camera.getView()), glm::vec2(1.f/extent.width, 1.f/extent.height));
				//Check this later
				//pointLightSystem.render(frameInfo);
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