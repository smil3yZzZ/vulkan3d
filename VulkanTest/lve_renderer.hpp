#pragma once

#include "lve_descriptors.hpp"
#include "lve_window.hpp"
#include "lve_device.hpp"
#include "lve_swap_chain.hpp"
#include "lve_buffer.hpp"

#include <cassert>
#include <memory>
#include <vector>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


namespace lve {
	class LveRenderer {
	public:
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

		VkDescriptorSetLayout getGBufferDescriptorSetLayout() { return gBufferSetLayout->getDescriptorSetLayout(); };
		VkDescriptorSetLayout getCompositionDescriptorSetLayout() { return compositionSetLayout->getDescriptorSetLayout(); };
		VkDescriptorSet getCurrentGBufferDescriptorSet() { return gBufferDescriptorSets[currentImageIndex]; };
		VkDescriptorSet getCurrentCompositionDescriptorSet() { return compositionDescriptorSets[currentImageIndex]; };
		void updateCurrentGBufferUbo(void* data);
		void updateCurrentCompositionUbo(void* data);

	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();
		void createDescriptorPool(int imageCount);
		void createUniformBuffers(int imageCount);

		LveWindow& lveWindow;
		LveDevice& lveDevice;
		LveAllocator& lveAllocator;
		std::unique_ptr<LveSwapChain> lveSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex;
		int currentFrameIndex{ 0 };
		bool isFrameStarted{false};

		std::unique_ptr<LveDescriptorSetLayout> gBufferSetLayout;
		std::unique_ptr<LveDescriptorSetLayout> compositionSetLayout;
		std::unique_ptr<LveDescriptorPool> globalPool;
		std::vector<std::unique_ptr<LveBuffer>> gBufferUboBuffers;
		std::vector<std::unique_ptr<LveBuffer>> compositionUboBuffers;
		std::vector<VkDescriptorSet> gBufferDescriptorSets;
		std::vector<VkDescriptorSet> compositionDescriptorSets;
	};
}