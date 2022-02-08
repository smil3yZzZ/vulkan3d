#include "lve_renderer.hpp"
#include <math.h>

#include <stdexcept>
#include <array>

#include <iostream>


namespace lve {

	LveRenderer::LveRenderer(LveWindow& window, LveDevice& device, LveAllocator& allocator) : lveWindow{ window }, lveDevice{ device }, lveAllocator{allocator} {
		recreateSwapChain();
		createCommandBuffers();
	}

	LveRenderer::~LveRenderer() {
		freeCommandBuffers();
	}

	void LveRenderer::recreateSwapChain() {
		auto extent = lveWindow.getExtent();
		while (extent.width == 0 || extent.height == 0) {
			extent = lveWindow.getExtent();
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(lveDevice.device());
		if (lveSwapChain == nullptr) {
			lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent);
			createDescriptorPool(lveSwapChain->imageCount());
		}
		else {
			globalPool->resetPool();
			std::shared_ptr<LveSwapChain> oldSwapChain = std::move(lveSwapChain);
			lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent, oldSwapChain);

			if (!oldSwapChain->compareSwapFormats(*lveSwapChain.get())) {
				throw std::runtime_error("Swap chain image(or depth) format has changed!");
			}
		}
		createUniformBuffers(lveSwapChain->imageCount());
	}

	void LveRenderer::createCommandBuffers() {
		commandBuffers.resize(LveSwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = lveDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw new std::runtime_error("failed to allocate command buffers");
		}
	}

	void LveRenderer::freeCommandBuffers() {
		vkFreeCommandBuffers(lveDevice.device(), lveDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
		commandBuffers.clear();
	}

	VkCommandBuffer LveRenderer::beginFrame() {
		assert(!isFrameStarted && "Can't call beginFrame while already in progress");

		auto result = lveSwapChain->acquireNextImage(&currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		isFrameStarted = true;

		auto commandBuffer = getCurrentCommandBuffer();
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer");
		}
		return commandBuffer;
	}

	void LveRenderer::endFrame() {
		assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
		auto commandBuffer = getCurrentCommandBuffer();

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}

		auto result = lveSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || lveWindow.wasWindowResized()) {
			lveWindow.resetWindowResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % LveSwapChain::MAX_FRAMES_IN_FLIGHT;
	}

	void LveRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = lveSwapChain->getRenderPass();
		renderPassInfo.framebuffer = lveSwapChain->getFrameBuffer(currentImageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 4> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[2].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[3].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(lveSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(lveSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, lveSwapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}
	void LveRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame");
		vkCmdEndRenderPass(commandBuffer);
	}

	void LveRenderer::createDescriptorPool(int imageCount) {
		globalPool = LveDescriptorPool::Builder(lveDevice)
			.setMaxSets(2 * imageCount)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2 * imageCount)
			.addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 3 * imageCount)
			.build();
	}

	void LveRenderer::createUniformBuffers(int imageCount) {
		gBufferUboBuffers.clear();
		compositionUboBuffers.clear();
		gBufferUboBuffers.resize(imageCount);
		compositionUboBuffers.resize(imageCount);

		for (int i = 0; i < imageCount; i++) {
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

		gBufferSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.build();

		gBufferDescriptorSets.clear();
		gBufferDescriptorSets.resize(imageCount);
		for (int i = 0; i < gBufferDescriptorSets.size(); i++) {
			auto bufferInfo = gBufferUboBuffers[i]->descriptorInfo();
			LveDescriptorWriter(*gBufferSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(gBufferDescriptorSets[i]);
		}

		compositionSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(2, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		compositionDescriptorSets.clear();
		compositionDescriptorSets.resize(imageCount);
		std::vector<LveSwapChain::Attachments> attachmentsVector = getSwapChainAttachments();
		for (int i = 0; i < compositionDescriptorSets.size(); i++) {
			auto bufferInfo = compositionUboBuffers[i]->descriptorInfo();
			auto normalInfo = attachmentsVector[i].normal.descriptorInfo();
			auto albedoInfo = attachmentsVector[i].albedo.descriptorInfo();
			auto depthInfo = attachmentsVector[i].depth.descriptorInfo();
			LveDescriptorWriter(*compositionSetLayout, *globalPool)
				.writeImage(0, &normalInfo)
				.writeImage(1, &albedoInfo)
				.writeImage(2, &depthInfo)
				.writeBuffer(3, &bufferInfo)
				.build(compositionDescriptorSets[i]);
		}
	}

	void LveRenderer::updateCurrentGBufferUbo(void* data) {
		gBufferUboBuffers[currentImageIndex]->writeToBuffer(data);
		gBufferUboBuffers[currentImageIndex]->flush();
	}
	void LveRenderer::updateCurrentCompositionUbo(void* data) {
		compositionUboBuffers[currentImageIndex]->writeToBuffer(data);
		compositionUboBuffers[currentImageIndex]->flush();
	}

}