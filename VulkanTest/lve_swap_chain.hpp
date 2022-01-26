#pragma once

#include "lve_device.hpp"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <string>
#include <vector>
#include <memory>

namespace lve {

class LveSwapChain {

 public:
    struct FrameBufferAttachment {
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        VkFormat format;
        VkDescriptorImageInfo descriptorInfo(VkSampler sampler = VK_NULL_HANDLE, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    };

    struct Attachments {
        FrameBufferAttachment position, normal, albedo, depth;
    };

  static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

  LveSwapChain(LveDevice &deviceRef, VkExtent2D windowExtent);
  LveSwapChain(LveDevice& deviceRef, VkExtent2D windowExtent, std::shared_ptr<LveSwapChain> previous);
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
  Attachments* getAttachments() { return &attachments; }

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

 private:
  void init();
  void createSwapChain();
  void createSwapChainImageViews();
  void createDeferredResources();
  void createAttachment(VkFormat format, VkImageUsageFlags usage, FrameBufferAttachment* attachment, VkExtent2D swapChainExtent);
  void createDepthResources();
  void createRenderPass();
  void createFramebuffers();
  void createSyncObjects();
  void destroyAttachment(FrameBufferAttachment* attachment);

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

  /*
  std::vector<VkImage> positionImages;
  std::vector<VkDeviceMemory> positionImageMemorys;
  std::vector<VkImageView> positionImageViews;
  std::vector<VkImage> normalImages;
  std::vector<VkDeviceMemory> normalImageMemorys;
  std::vector<VkImageView> normalImageViews;
  std::vector<VkImage> albedoImages;
  std::vector<VkDeviceMemory> albedoImageMemorys;
  std::vector<VkImageView> albedoImageViews;
  std::vector<VkImage> depthImages;
  std::vector<VkDeviceMemory> depthImageMemorys;
  std::vector<VkImageView> depthImageViews;
  */
  Attachments attachments;
  std::vector<VkImage> swapChainImages;
  std::vector<VkImageView> swapChainImageViews;

  LveDevice &device;
  VkExtent2D windowExtent;

  VkSwapchainKHR swapChain;
  std::shared_ptr<LveSwapChain> oldSwapChain;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  std::vector<VkFence> imagesInFlight;
  size_t currentFrame = 0;
};

}  // namespace lve
