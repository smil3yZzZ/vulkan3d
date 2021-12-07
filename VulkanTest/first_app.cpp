#include "first_app.hpp"
#include <math.h>

#include <stdexcept>
#include <array>

#include <iostream>


namespace lve {

	FirstApp::FirstApp() {
		createPipelineLayout();
		recreateSwapChain();
		loadModels();
		createCommandBuffers();
	}

	FirstApp::~FirstApp() {
		vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
	}

	void FirstApp::run() {
		int frame = 0;
		int powIteration = 1;
		while (!lveWindow.shouldClose()) {
			if (frame > 0 && frame % 60 == 0 && frame < 60 * SIERPINSKI_DEPTH) {
				updateModels(powIteration++);
			}
			glfwPollEvents();
			drawFrame();
			frame++;
		}
		vkDeviceWaitIdle(lveDevice.device());
	}

	void FirstApp::loadModels() {
		vertices = {
			{{0.0f, -0.5f}, {1.0f, 1.0f, 0.0f, 0.5f}},
			{{0.5f, 0.5f}, {1.0f, 1.0f, 0.0f, 0.5f}},
			{{-0.5f, 0.5f}, {1.0f, 1.0f, 0.0f, 0.5f}}
		};
		lveModel = std::make_unique<LveModel>(lveSwapChain->getCurrentFrame(), lveDevice, vertices, lveAllocator);
	}

	void FirstApp::updateModels(int powIteration) {
		int numOfVertices = NUMBER_OF_TRIANGLE_VERTICES;
		int previousNumOfTriangles = pow(numOfVertices, (double)powIteration - 1);
		int numOfTrianglesPerPreviousTriangles = pow(numOfVertices, (double)powIteration)/previousNumOfTriangles;
		std::vector<LveModel::Vertex> auxVertices(numOfVertices * previousNumOfTriangles * numOfTrianglesPerPreviousTriangles);
		for (int i = 0; i < previousNumOfTriangles; i++) {
			for (int j = 0; j < numOfTrianglesPerPreviousTriangles; j++) {
				for (int k = 0; k < numOfVertices; k++) {
					auxVertices[i * numOfTrianglesPerPreviousTriangles * numOfVertices + j * numOfVertices + k].position = (vertices[i * numOfVertices + j].position + vertices[i * numOfVertices + k].position) / 2.0f;
					auxVertices[i * numOfTrianglesPerPreviousTriangles * numOfVertices + j * numOfVertices + k].color = vertices[i * numOfVertices + j].color;
					auxVertices[i * numOfTrianglesPerPreviousTriangles * numOfVertices + j * numOfVertices + k].color[3] = 0.5f;
				}
			}
		}
		vertices = auxVertices;
	}

	void FirstApp::createPipelineLayout() {
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
		if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void FirstApp::createPipeline() {
		assert(lveSwapChain != nullptr && "Cannot create pipeline before swap chain");
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
		PipelineConfigInfo pipelineConfig{};
		LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = lveSwapChain->getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		lvePipeline = std::make_unique<LvePipeline>(
			lveDevice,
			"shaders/simple_shader.vert.spv",
			"shaders/simple_shader.frag.spv",
			pipelineConfig
			);
	}

	void FirstApp::recreateSwapChain() {
		auto extent = lveWindow.getExtent();
		while (extent.width == 0 || extent.height == 0) {
			extent = lveWindow.getExtent();
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(lveDevice.device());
		if (lveSwapChain == nullptr) {
			lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent);
		} else {
			lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent, std::move(lveSwapChain));
			if (lveSwapChain->imageCount() != commandBuffers.size()) {
				freeCommandBuffers();
				createCommandBuffers();
			}
		}
		createPipeline();
	}

	void FirstApp::createCommandBuffers() {
		commandBuffers.resize(lveSwapChain->imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = lveDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw new std::runtime_error("failed to allocate command buffers");
		}
	}

	void FirstApp::freeCommandBuffers() {
		vkFreeCommandBuffers(lveDevice.device(), lveDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
		commandBuffers.clear();
	}

	void FirstApp::recordCommandBuffer(size_t vertexBufferIndex, int imageIndex) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = lveSwapChain->getRenderPass();
		renderPassInfo.framebuffer = lveSwapChain->getFrameBuffer(imageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(lveSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(lveSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, lveSwapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
		vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

		lvePipeline->bind(commandBuffers[imageIndex]);
		lveModel->bind(vertexBufferIndex, commandBuffers[imageIndex]);
		lveModel->draw(commandBuffers[imageIndex]);

		vkCmdEndRenderPass(commandBuffers[imageIndex]);
		if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	
	void FirstApp::drawFrame() {
		uint32_t imageIndex;
		auto result = lveSwapChain->acquireNextImage(&imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			std::cout << "Recreating swapchain..." << std::endl;
			recreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		lveModel->updateVertexBufferData(lveSwapChain->getCurrentFrame(), vertices);
		recordCommandBuffer(lveSwapChain->getCurrentFrame(), imageIndex);

		result = lveSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || lveWindow.wasWindowResized()) {
			std::cout << "Recreating swapchain and resetting window resized flag..." << std::endl;
			lveWindow.resetWindowResizedFlag();
			recreateSwapChain();
			return;
		}
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}
	}

}