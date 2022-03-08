#pragma once

#include "vk3d_descriptors.hpp"
#include "vk3d_window.hpp"
#include "vk3d_device.hpp"
#include "vk3d_swap_chain.hpp"
#include "vk3d_buffer.hpp"

#include <cassert>
#include <vector>


namespace vk3d {
	class Vk3dRenderer {
	public:
		Vk3dRenderer(Vk3dWindow &window, Vk3dDevice &device, Vk3dAllocator &allocator);
		~Vk3dRenderer();

		Vk3dRenderer(const Vk3dRenderer&) = delete;
		Vk3dRenderer& operator=(const Vk3dRenderer&) = delete;

		VkRenderPass getSwapChainRenderPass() const { return lveSwapChain->getRenderPass(); }
		VkRenderPass getShadowRenderPass() const { return lveSwapChain->getShadowRenderPass(); }
		float getAspectRatio() const { return lveSwapChain->extentAspectRatio(); };
		float getShadowAspectRatio() const { return lveSwapChain->shadowExtentAspectRatio(); };
		VkExtent2D getExtent() const { return lveSwapChain->getSwapChainExtent(); };
		std::vector<Vk3dSwapChain::Attachments> getSwapChainAttachments() { return lveSwapChain->getAttachments(); };
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
		void beginShadowRenderPass(VkCommandBuffer commandBuffer);
		void endShadowRenderPass(VkCommandBuffer commandBuffer);
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

		VkDescriptorSetLayout getShadowDescriptorSetLayout() { return lveSwapChain->getShadowDescriptorSetLayout(); };
		VkDescriptorSetLayout getGBufferDescriptorSetLayout() { return lveSwapChain->getGBufferDescriptorSetLayout(); };
		VkDescriptorSetLayout getCompositionDescriptorSetLayout() { return lveSwapChain->getCompositionDescriptorSetLayout(); };
		VkDescriptorSet getCurrentShadowDescriptorSet() { return lveSwapChain->getCurrentShadowDescriptorSet(currentImageIndex); };
		VkDescriptorSet getCurrentGBufferDescriptorSet() { return lveSwapChain->getCurrentGBufferDescriptorSet(currentImageIndex); };
		VkDescriptorSet getCurrentCompositionDescriptorSet() { return lveSwapChain->getCurrentCompositionDescriptorSet(currentImageIndex);};
		void updateCurrentShadowUbo(void* data) { return lveSwapChain->updateCurrentShadowUbo(data, currentImageIndex); };
		void updateCurrentGBufferUbo(void* data) { return lveSwapChain->updateCurrentGBufferUbo(data, currentImageIndex); };
		void updateCurrentCompositionUbo(void* data) { return lveSwapChain->updateCurrentCompositionUbo(data, currentImageIndex); };

	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();

		Vk3dWindow& lveWindow;
		Vk3dDevice& lveDevice;
		Vk3dAllocator& lveAllocator;
		std::unique_ptr<Vk3dSwapChain> lveSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex;
		int currentFrameIndex{ 0 };
		bool isFrameStarted{false};
	};
}