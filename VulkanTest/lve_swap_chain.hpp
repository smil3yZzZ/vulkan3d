#pragma once

#include "lve_descriptors.hpp"
#include "lve_device.hpp"
#include "lve_buffer.hpp"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <string>
#include <vector>
#include <memory>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace lve {

class LveSwapChain {

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

    struct FrameBufferAttachment {
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        VkFormat format;
        VkDescriptorImageInfo descriptorInfo(VkSampler sampler = VK_NULL_HANDLE, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    };

    struct Sampler {
        VkSampler sampler;
        FrameBufferAttachment attachment;
    };

    struct Samplers {
        Sampler shadowDepth;
    };

    struct Attachments {
        FrameBufferAttachment normal, albedo, depth;
    };

  static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

  LveSwapChain(LveDevice &deviceRef, LveAllocator& allocatorRef, VkExtent2D windowExtent);
  LveSwapChain(LveDevice& deviceRef, LveAllocator& allocatorRef, VkExtent2D windowExtent, std::shared_ptr<LveSwapChain> previous);
  ~LveSwapChain();

  LveSwapChain(const LveSwapChain &) = delete;
  LveSwapChain& operator=(const LveSwapChain &) = delete;

  VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
  VkRenderPass getRenderPass() { return renderPass; }
  VkImageView getImageView(int index) { return swapChainImageViews[index]; }
  size_t imageCount() { return swapChainImages.size(); }
  VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
  VkExtent2D getSwapChainExtent() { return swapChainExtent; }
  uint32_t width() { return swapChainExtent.width; }
  uint32_t height() { return swapChainExtent.height; }
  std::vector<Attachments> getAttachments() { return attachmentsVector; }

  float extentAspectRatio() {
    return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
  }
  VkFormat findDepthFormat();

  VkResult acquireNextImage(uint32_t *imageIndex);
  VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

  bool compareSwapFormats(const LveSwapChain &swapChain) const {
      return swapChain.swapChainDepthFormat == swapChainDepthFormat &&
          swapChain.swapChainImageFormat == swapChainImageFormat;
  }

  size_t getCurrentFrame() { return currentFrame; }

  VkDescriptorSetLayout getGBufferDescriptorSetLayout() { return gBufferSetLayout->getDescriptorSetLayout(); };
  VkDescriptorSetLayout getCompositionDescriptorSetLayout() { return compositionSetLayout->getDescriptorSetLayout(); };
  VkDescriptorSet getCurrentGBufferDescriptorSet(int currentImageIndex) { return gBufferDescriptorSets[currentImageIndex]; };
  VkDescriptorSet getCurrentCompositionDescriptorSet(int currentImageIndex) { return compositionDescriptorSets[currentImageIndex]; };
  void updateCurrentGBufferUbo(void* data, int currentImageIndex);
  void updateCurrentCompositionUbo(void* data, int currentImageIndex);

 private:
  void init();
  void createSwapChain();
  void createSwapChainImageViews();
  void createDeferredResources();
  void createAttachment(VkFormat format, VkImageUsageFlags usage, FrameBufferAttachment* attachment, VkExtent2D swapChainExtent);
  void createSampler(VkFormat format, VkImageUsageFlags usage, FrameBufferAttachment* attachment, VkExtent2D swapChainExtent);
  void createCompositionRenderPass();
  void createShadowRenderPass();
  void createCompositionFramebuffers();
  void createShadowFramebuffers();
  void createSyncObjects();
  void destroyAttachment(FrameBufferAttachment* attachment);
  void destroySampler(Sampler* sampler);
  void createDescriptorPool();
  void createUniformBuffers();

  // Helper functions
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

  VkFormat swapChainImageFormat;
  VkFormat swapChainDepthFormat;
  VkFormat deferredResourcesFormat;
  VkExtent2D swapChainExtent;

  std::vector<VkFramebuffer> swapChainFramebuffers;
  VkRenderPass renderPass;

  std::vector<Attachments> attachmentsVector;
  std::vector<VkImage> swapChainImages;
  std::vector<VkImageView> swapChainImageViews;

  LveDevice &device;
  LveAllocator &allocator;
  VkExtent2D windowExtent;

  VkSwapchainKHR swapChain;
  std::shared_ptr<LveSwapChain> oldSwapChain;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  std::vector<VkFence> imagesInFlight;
  size_t currentFrame = 0;

  std::unique_ptr<LveDescriptorSetLayout> gBufferSetLayout;
  std::unique_ptr<LveDescriptorSetLayout> compositionSetLayout;
  std::unique_ptr<LveDescriptorPool> globalPool;
  std::vector<std::unique_ptr<LveBuffer>> gBufferUboBuffers;
  std::vector<std::unique_ptr<LveBuffer>> compositionUboBuffers;
  std::vector<VkDescriptorSet> gBufferDescriptorSets;
  std::vector<VkDescriptorSet> compositionDescriptorSets;
};

}  // namespace lve
