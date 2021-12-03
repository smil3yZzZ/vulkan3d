#include "first_app.hpp"
#include <math.h>

#include <stdexcept>
#include <array>

#include <iostream>


namespace lve {

	FirstApp::FirstApp() {
		loadModels();
		createPipelineLayout();
		createPipeline();
		initCommandBuffers();
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
			{{0.0f, -0.5f}},
			{{0.5f, 0.5f}},
			{{-0.5f, 0.5f}}
		};
		lveModel = std::make_unique<LveModel>(lveSwapChain.getCurrentFrame(), lveDevice, vertices, lveAllocator);
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
		PipelineConfigInfo pipelineConfig{};
		LvePipeline::defaultPipelineConfigInfo(pipelineConfig, lveSwapChain.width(), lveSwapChain.height());
		pipelineConfig.renderPass = lveSwapChain.getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		lvePipeline = std::make_unique<LvePipeline>(
			lveDevice,
			"shaders/simple_shader.vert.spv",
			"shaders/simple_shader.frag.spv",
			pipelineConfig
			);
	}

	void FirstApp::initCommandBuffers() {
		commandBuffers.resize(lveSwapChain.imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = lveDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw new std::runtime_error("failed to allocate command buffers");
		}
	}

	void FirstApp::createCommandBuffer(size_t vertexBufferIndex) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffers[currentIndex], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = lveSwapChain.getRenderPass();
		renderPassInfo.framebuffer = lveSwapChain.getFrameBuffer(currentIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = lveSwapChain.getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffers[currentIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		lvePipeline->bind(commandBuffers[currentIndex]);
		lveModel->bind(vertexBufferIndex, commandBuffers[currentIndex]);
		lveModel->draw(commandBuffers[currentIndex]);

		vkCmdEndRenderPass(commandBuffers[currentIndex]);
		if (vkEndCommandBuffer(commandBuffers[currentIndex]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}

		currentIndex = (currentIndex + 1) % commandBuffers.size();
	}

	
	void FirstApp::drawFrame() {
		uint32_t imageIndex;
		auto result = lveSwapChain.acquireNextImage(&imageIndex);

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		lveModel->updateVertexBufferData(lveSwapChain.getCurrentFrame(), vertices);
		createCommandBuffer(lveSwapChain.getCurrentFrame());

		result = lveSwapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}
	}

}