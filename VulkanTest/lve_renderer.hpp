#pragma once

#include "lve_descriptors.hpp"
#include "lve_window.hpp"
#include "lve_device.hpp"
#include "lve_swap_chain.hpp"
#include "lve_buffer.hpp"
#include "lve_frame_info.hpp"

#include <cassert>
#include <vector>


namespace lve {
	class LveRenderer {
	public:
		static constexpr int NUM_CUBE_FACES = 6;

		LveRenderer(LveWindow &window, LveDevice &device, LveAllocator &allocator);
		~LveRenderer();

		LveRenderer(const LveRenderer&) = delete;
		LveRenderer& operator=(const LveRenderer&) = delete;


		VkRenderPass getSwapChainRenderPass() const { return lveSwapChain->getRenderPass(); }
		VkRenderPass getShadowRenderPass() const { return lveSwapChain->getShadowRenderPass(); }
		float getAspectRatio() const { return lveSwapChain->extentAspectRatio(); };
		float getShadowAspectRatio() const { return lveSwapChain->shadowExtentAspectRatio(); };
		VkExtent2D getExtent() const { return lveSwapChain->getSwapChainExtent(); };
		VkExtent2D getShadowMapExtent() const { return lveSwapChain->getShadowMapExtent(); };
		LveSwapChain::Attachments getAttachments() { return lveSwapChain->getAttachments(currentImageIndex); };
		LveSwapChain::Samplers getSamplers() { return lveSwapChain->getSamplers(currentImageIndex); };
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
		void beginShadowRenderPassConfig(VkCommandBuffer commandBuffer);
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
		void createMainCommandBuffers();
		void createShadowCubeCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();

		LveWindow& lveWindow;
		LveDevice& lveDevice;
		LveAllocator& lveAllocator;
		std::unique_ptr<LveSwapChain> lveSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;
		std::vector<std::vector<VkCommandBuffer>> shadowCubeCommandBuffers;

		uint32_t currentImageIndex;
		int currentFrameIndex{ 0 };
		bool isFrameStarted{false};
	};
}