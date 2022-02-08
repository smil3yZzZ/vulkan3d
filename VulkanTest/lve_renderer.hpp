#pragma once

#include "lve_descriptors.hpp"
#include "lve_window.hpp"
#include "lve_device.hpp"
#include "lve_swap_chain.hpp"
#include "lve_buffer.hpp"

#include <cassert>
#include <vector>


namespace lve {
	class LveRenderer {
	public:
		LveRenderer(LveWindow &window, LveDevice &device, LveAllocator &allocator);
		~LveRenderer();

		LveRenderer(const LveRenderer&) = delete;
		LveRenderer& operator=(const LveRenderer&) = delete;

		VkRenderPass getSwapChainRenderPass() const { return lveSwapChain->getRenderPass(); }
		float getAspectRatio() const { return lveSwapChain->extentAspectRatio(); };
		VkExtent2D getExtent() const { return lveSwapChain->getSwapChainExtent(); };
		std::vector<LveSwapChain::Attachments> getSwapChainAttachments() { return lveSwapChain->getAttachments(); };
		bool isFrameInProgress() const { return isFrameStarted; }

		VkCommandBuffer getCurrentCommandBuffer() const {
			assert(isFrameStarted && "Cannot get command buffer when frame is not in progress");
			return commandBuffers[currentFrameIndex];
		}

		int getFrameIndex() const {
			assert(isFrameStarted && "Cannot get frame index when frame not in progress");
			return currentFrameIndex;
		}

		size_t getCurrentFrame() { return lveSwapChain->getCurrentFrame(); }
		size_t getCurrentImageIndex() { return currentImageIndex; }

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

		VkDescriptorSetLayout getGBufferDescriptorSetLayout() { return lveSwapChain->getGBufferDescriptorSetLayout(); };
		VkDescriptorSetLayout getCompositionDescriptorSetLayout() { return lveSwapChain->getCompositionDescriptorSetLayout(); };
		VkDescriptorSet getCurrentGBufferDescriptorSet() { return lveSwapChain->getCurrentGBufferDescriptorSet(currentImageIndex); };
		VkDescriptorSet getCurrentCompositionDescriptorSet() { return lveSwapChain->getCurrentCompositionDescriptorSet(currentImageIndex);};
		void updateCurrentGBufferUbo(void* data) { return lveSwapChain->updateCurrentGBufferUbo(data, currentImageIndex); };
		void updateCurrentCompositionUbo(void* data) { return lveSwapChain->updateCurrentCompositionUbo(data, currentImageIndex); };

	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();

		LveWindow& lveWindow;
		LveDevice& lveDevice;
		LveAllocator& lveAllocator;
		std::unique_ptr<LveSwapChain> lveSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex;
		int currentFrameIndex{ 0 };
		bool isFrameStarted{false};
	};
}