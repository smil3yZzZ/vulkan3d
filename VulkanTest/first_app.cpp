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
		createCommandBuffers();
	}

	FirstApp::~FirstApp() {
		vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
	}

	void FirstApp::run() {
		int frame = 0;
		int powIteration = 1;
		while (!lveWindow.shouldClose()) {
			if (frame > 60 && frame % 60 == 0) {
				std::cout << "Frame: " << frame << std::endl;
				updateModels(powIteration++);
			}
			//createCommandBuffer();
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
		lveModel = std::make_unique<LveModel>(lveDevice, vertices);
	}

	void FirstApp::updateModels(int powIteration) {
		int numOfVertices = NUMBER_OF_TRIANGLE_VERTICES;
		//std::cout << "Update mode:" << std::endl;
		int previousNumOfTriangles = pow(numOfVertices, (double)powIteration - 1);
		int numOfTrianglesPerPreviousTriangles = pow(numOfVertices, (double)powIteration)/previousNumOfTriangles;
		std::vector<LveModel::Vertex> auxVertices(numOfVertices * previousNumOfTriangles * numOfTrianglesPerPreviousTriangles);
		//std::cout << "Update mode after pow:" << std::endl;
		for (int i = 0; i < previousNumOfTriangles; i++) {
			for (int j = 0; j < numOfTrianglesPerPreviousTriangles; j++) {
				for (int k = 0; k < numOfVertices; k++) {
					//std::cout << "Current triangle (previous): " << i << " - Current triangle per previous triangle: " << j << " - Current vertex: " << k << std::endl;
					//std::cout << "Vertices size: " << vertices.size() << std::endl;
					auxVertices[i * numOfTrianglesPerPreviousTriangles * numOfVertices + j * numOfVertices + k].position = (vertices[i * numOfVertices + j].position + vertices[i * numOfVertices + k].position) / 2.0f;
					//std::cout << auxVertices[i * numOfTrianglesPerPreviousTriangles * numOfVertices + j * numOfVertices + k].position[0] << "," << auxVertices[i * numOfTrianglesPerPreviousTriangles * numOfVertices + j * numOfVertices + k].position[1] << std::endl;
					//auxVertices[i * numOfVertices + j] = (vertices[(i / numOfTriangles) * numOfVertices + i].position + vertices[(i / numOfTriangles) * numOfVertices + j].position)/(powIteration + 1);
				}
			}
		}
		vertices = auxVertices;
		
		//lveModel->updateVertexBuffersData(vertices);
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

	void FirstApp::createCommandBuffers() {
		commandBuffers.resize(lveSwapChain.imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = lveDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw new std::runtime_error("failed to allocate command buffers");
		}

		for (int i = 0; i < commandBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
				throw std::runtime_error("failed to begin recording command buffer");
			}

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = lveSwapChain.getRenderPass();
			renderPassInfo.framebuffer = lveSwapChain.getFrameBuffer(i);

			renderPassInfo.renderArea.offset = {0, 0};
			renderPassInfo.renderArea.extent = lveSwapChain.getSwapChainExtent();

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
			clearValues[1].depthStencil = {1.0f, 0};
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			lvePipeline->bind(commandBuffers[i]);
			lveModel->bind(commandBuffers[i]);
			lveModel->draw(commandBuffers[i]);

			vkCmdEndRenderPass(commandBuffers[i]);
			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}
		}
	}
	void FirstApp::drawFrame() {
		uint32_t imageIndex;
		auto result = lveSwapChain.acquireNextImage(&imageIndex);

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		result = lveSwapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}
	}

}