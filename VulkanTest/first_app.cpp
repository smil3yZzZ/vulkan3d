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
#include <numeric>
#include <array>
#include <chrono>

#include <iostream>


namespace lve {

	struct GlobalUbo {
		glm::mat4 projectionView{ 1.f };
		glm::vec3 lightDirection = glm::normalize(glm::vec3{1.f, -3.f, -1.f});
	};

	FirstApp::FirstApp() {
		globalPool = LveDescriptorPool::Builder(lveDevice)
			.setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();
		loadGameObjects();
	}

	FirstApp::~FirstApp() {
	}

	void FirstApp::run() {
		std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++) {
			uboBuffers[i] = std::make_unique<LveBuffer>(
				lveDevice,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VMA_MEMORY_USAGE_CPU_TO_GPU,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
				lveAllocator
			);
			uboBuffers[i]->map();
		}

		auto globalSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.build();

		std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++) {
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			LveDescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSets[i]);
		}

		SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
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
				int frameIndex = lveRenderer.getFrameIndex();
				FrameInfo frameInfo{
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					globalDescriptorSets[frameIndex]
				};
				// update
				GlobalUbo ubo{};
				ubo.projectionView = camera.getProjection() * camera.getView();
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();

				// render
				lveRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(frameInfo, gameObjects);
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