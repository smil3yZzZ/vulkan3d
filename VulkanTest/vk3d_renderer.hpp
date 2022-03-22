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

		VkRenderPass getSwapChainRenderPass() const { return vk3dSwapChain->getRenderPass(); }
		VkRenderPass getShadowRenderPass() const { return vk3dSwapChain->getShadowRenderPass(); }
		VkRenderPass getMappingsRenderPass() const { return vk3dSwapChain->getMappingsRenderPass(); }
		float getAspectRatio() const { return vk3dSwapChain->extentAspectRatio(); };
		float getShadowAspectRatio() const { return vk3dSwapChain->shadowExtentAspectRatio(); };
		VkExtent2D getExtent() const { return vk3dSwapChain->getSwapChainExtent(); };
		std::vector<Vk3dSwapChain::Attachments> getSwapChainAttachments() { return vk3dSwapChain->getAttachments(); };
		bool isFrameInProgress() const { return isFrameStarted; }

		VkCommandBuffer getCurrentCommandBuffer() const {
			assert(isFrameStarted && "Cannot get command buffer when frame is not in progress");
			return commandBuffers[currentFrameIndex];
		}

		int getFrameIndex() const {
			assert(isFrameStarted && "Cannot get frame index when frame not in progress");
			return currentFrameIndex;
		}

		size_t getCurrentFrame() { return vk3dSwapChain->getCurrentFrame(); }
		size_t getCurrentImageIndex() { return currentImageIndex; }

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginShadowRenderPass(VkCommandBuffer commandBuffer);
		void endShadowRenderPass(VkCommandBuffer commandBuffer);
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void beginMappingsRenderPass(VkCommandBuffer commandBuffer);
		void endMappingsRenderPass(VkCommandBuffer commandBuffer);

		VkDescriptorSetLayout getShadowDescriptorSetLayout() { return vk3dSwapChain->getShadowDescriptorSetLayout(); };
		VkDescriptorSetLayout getGBufferDescriptorSetLayout() { return vk3dSwapChain->getGBufferDescriptorSetLayout(); };
		VkDescriptorSetLayout getCompositionDescriptorSetLayout() { return vk3dSwapChain->getCompositionDescriptorSetLayout(); };
		VkDescriptorSetLayout getMappingsDescriptorSetLayout() { return vk3dSwapChain->getMappingsDescriptorSetLayout(); };
		VkDescriptorSet getCurrentShadowDescriptorSet() { return vk3dSwapChain->getCurrentShadowDescriptorSet(currentImageIndex); };
		VkDescriptorSet getCurrentGBufferDescriptorSet() { return vk3dSwapChain->getCurrentGBufferDescriptorSet(currentImageIndex); };
		VkDescriptorSet getCurrentCompositionDescriptorSet() { return vk3dSwapChain->getCurrentCompositionDescriptorSet(currentImageIndex);};
		VkDescriptorSet getCurrentMappingsDescriptorSet() { return vk3dSwapChain->getCurrentMappingsDescriptorSet(currentImageIndex); };
		void updateCurrentShadowUbo(void* data) { return vk3dSwapChain->updateCurrentShadowUbo(data, currentImageIndex); };
		void updateCurrentGBufferUbo(void* data) { return vk3dSwapChain->updateCurrentGBufferUbo(data, currentImageIndex); };
		void updateCurrentCompositionUbo(void* data) { return vk3dSwapChain->updateCurrentCompositionUbo(data, currentImageIndex); };
		void updateCurrentMappingsUbo(void* data) { return vk3dSwapChain->updateCurrentMappingsUbo(data, currentImageIndex); };

	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();

		Vk3dWindow& vk3dWindow;
		Vk3dDevice& vk3dDevice;
		Vk3dAllocator& vk3dAllocator;
		std::unique_ptr<Vk3dSwapChain> vk3dSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex;
		int currentFrameIndex{ 0 };
		bool isFrameStarted{false};
	};
}