#pragma once

#include "vk3d_descriptors.hpp"
#include "vk3d_device.hpp"
#include "vk3d_buffer.hpp"

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

namespace vk3d {

class Vk3dSwapChain {

 public:
     struct ShadowUbo {
         glm::mat4 projectionView[6];
         glm::vec3 lightPosition{ LIGHT_POSITION };
     };

     struct MappingsUbo {
         glm::mat4 projection{ 1.f };
         glm::mat4 view{ 1.f };
     };

     struct UVReflectionUbo {
         glm::vec3 viewPos;
         alignas(16) glm::mat4 projection{ 1.f };
         glm::mat4 view{ 1.f };
         glm::vec2 invResolution;
     };

     struct GBufferUbo {
         glm::mat4 projection{ 1.f };
         glm::mat4 view{ 1.f };
         glm::vec3 lightPosition{ LIGHT_POSITION };
     };

     struct CompositionUbo {
         glm::vec3 viewPos;
         alignas(16) glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .15f }; //w is intensity
         glm::vec3 lightPosition{ LIGHT_POSITION };
         alignas(16) glm::vec4 lightColor{ .8f, 1.f, .2f, 1.f }; //w is light intensity
     };

     struct PostProcessingUbo {
         glm::vec2 invResolution;
     };

    struct FrameBufferAttachment {
        VkImage image = VK_NULL_HANDLE;
        VmaAllocation memory = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        VkFormat format;
        VkDescriptorImageInfo descriptorInfo(VkSampler sampler = VK_NULL_HANDLE, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    };

    struct ShadowMapDimension {
        int width, height;
    };

    struct Sampler {
        VkSampler sampler;
        FrameBufferAttachment attachment;
    };

    struct Samplers {
        Sampler shadowOmniMap, mappingsMap, uvReflectionMap, lightingMap;
    };

    struct Attachments {
        FrameBufferAttachment normal, albedo, depth, shadowDepth, mappingsMapDepth, uvReflectionMapDepth;
    };

  static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

  static constexpr int SHADOW_MAP_WIDTH = 1024;
  static constexpr int SHADOW_MAP_HEIGHT = 1024;

  static constexpr int NUM_CUBE_FACES = 6;
  static constexpr int MAPPINGS_ARRAY_LENGTH = 2;

  static constexpr VkFormat SHADOW_FB_COLOR_FORMAT = VK_FORMAT_R32_SFLOAT;
  static constexpr VkFormat DEFERRED_RESOURCES_FORMAT = VK_FORMAT_R32G32B32A32_SFLOAT;

  static constexpr glm::vec3 LIGHT_POSITION = glm::vec3{ 1.f, -4.f, -4.f };

  static constexpr glm::vec3 CAMERA_POSITION = glm::vec3{ 1.f, -1.f, -4.f };

  static constexpr VkFilter DEFAULT_SHADOWMAP_FILTER = VK_FILTER_LINEAR;

  Vk3dSwapChain(Vk3dDevice &deviceRef, Vk3dAllocator& allocatorRef, VkExtent2D windowExtent);
  Vk3dSwapChain(Vk3dDevice& deviceRef, Vk3dAllocator& allocatorRef, VkExtent2D windowExtent, std::shared_ptr<Vk3dSwapChain> previous);
  ~Vk3dSwapChain();

  Vk3dSwapChain(const Vk3dSwapChain &) = delete;
  Vk3dSwapChain& operator=(const Vk3dSwapChain &) = delete;

  VkFramebuffer getShadowFrameBuffer(int index) { return shadowFramebuffers[index]; }
  VkFramebuffer getMappingsFrameBuffer(int index) { return mappingsFramebuffers[index]; }
  VkFramebuffer getUVReflectionFrameBuffer(int index) { return uvReflectionFramebuffers[index]; }
  VkFramebuffer getLightingFrameBuffer(int index) { return lightingFramebuffers[index]; }
  VkFramebuffer getPostProcessingFrameBuffer(int index) { return postProcessingFramebuffers[index]; }
  VkRenderPass getShadowRenderPass() { return shadowRenderPass; }
  VkRenderPass getMappingsRenderPass() { return mappingsRenderPass; }
  VkRenderPass getUVReflectionRenderPass() { return uvReflectionRenderPass; }
  VkRenderPass getLightingRenderPass() { return lightingRenderPass; }
  VkRenderPass getPostProcessingRenderPass() { return postProcessingRenderPass; }
  VkImageView getImageView(int index) { return swapChainImageViews[index]; }
  size_t imageCount() { return swapChainImages.size(); }
  VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
  VkExtent2D getSwapChainExtent() { return swapChainExtent; }
  uint32_t width() { return swapChainExtent.width; }
  uint32_t height() { return swapChainExtent.height; }
  std::vector<Attachments> getAttachments() { return attachmentsVector; }
  VkExtent2D getShadowMapExtent() { return VkExtent2D{SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT}; }

  float extentAspectRatio() {
    return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
  }
  float shadowExtentAspectRatio() {
      return static_cast<float>(getShadowMapExtent().width) / static_cast<float>(getShadowMapExtent().height);
  }
  VkFormat findDepthFormat();

  VkResult acquireNextImage(uint32_t *imageIndex);
  VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

  bool compareSwapFormats(const Vk3dSwapChain &swapChain) const {
      return swapChain.swapChainDepthFormat == swapChainDepthFormat &&
          swapChain.swapChainImageFormat == swapChainImageFormat;
  }

  size_t getCurrentFrame() { return currentFrame; }

  VkDescriptorSetLayout getShadowDescriptorSetLayout() { return shadowSetLayout->getDescriptorSetLayout(); };
  VkDescriptorSetLayout getMappingsDescriptorSetLayout() { return mappingsSetLayout->getDescriptorSetLayout(); };
  VkDescriptorSetLayout getUVReflectionDescriptorSetLayout() { return uvReflectionSetLayout->getDescriptorSetLayout(); };
  VkDescriptorSetLayout getGBufferDescriptorSetLayout() { return gBufferSetLayout->getDescriptorSetLayout(); };
  VkDescriptorSetLayout getCompositionDescriptorSetLayout() { return compositionSetLayout->getDescriptorSetLayout(); };
  VkDescriptorSetLayout getPostProcessingDescriptorSetLayout() { return postProcessingSetLayout->getDescriptorSetLayout(); };
  VkDescriptorSet getCurrentShadowDescriptorSet(int currentImageIndex) { return shadowDescriptorSets[currentImageIndex]; };
  VkDescriptorSet getCurrentMappingsDescriptorSet(int currentImageIndex) { return mappingsDescriptorSets[currentImageIndex]; };
  VkDescriptorSet getCurrentUVReflectionDescriptorSet(int currentImageIndex) { return uvReflectionDescriptorSets[currentImageIndex]; };
  VkDescriptorSet getCurrentGBufferDescriptorSet(int currentImageIndex) { return gBufferDescriptorSets[currentImageIndex]; };
  VkDescriptorSet getCurrentCompositionDescriptorSet(int currentImageIndex) { return compositionDescriptorSets[currentImageIndex]; };
  VkDescriptorSet getCurrentPostProcessingDescriptorSet(int currentImageIndex) { return postProcessingDescriptorSets[currentImageIndex]; };
  void updateCurrentShadowUbo(void* data, int currentImageIndex);
  void updateCurrentMappingsUbo(void* data, int currentImageIndex);
  void updateCurrentUVReflectionUbo(void* data, int currentImageIndex);
  void updateCurrentGBufferUbo(void* data, int currentImageIndex);
  void updateCurrentCompositionUbo(void* data, int currentImageIndex);
  void updateCurrentPostProcessingUbo(void* data, int currentImageIndex);

 private:
  void init();
  void createSwapChain();
  void createSwapChainImageViews();
  void createSampler(VkFormat format, VkImageUsageFlags usage, Sampler* attachment, VkExtent2D extent, VkImageViewType imageViewType = VK_IMAGE_VIEW_TYPE_2D, uint32_t arrayLayers = 1);
  void createAttachment(VkFormat format, VkImageUsageFlags usage, FrameBufferAttachment* attachment, VkExtent2D extent, VkImageViewType imageViewType = VK_IMAGE_VIEW_TYPE_2D, uint32_t arrayLayers = 1);
  void createShadowSampler();
  void createShadowRenderPass();
  void createShadowFramebuffers();
  void createMappingsSampler();
  void createMappingsRenderPass();
  void createMappingsFramebuffers();
  void createUVMapSampler();
  void createUVMapRenderPass();
  void createUVMapFramebuffers();
  void createDeferredResources();
  void createLightingRenderPass();
  void createLightingFramebuffers();
  void createPostProcessingRenderPass();
  void createPostProcessingFramebuffers();
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
  VkBool32 formatIsFilterable(VkPhysicalDevice physicalDevice, VkFormat format, VkImageTiling tiling);

  VkFormat swapChainImageFormat;
  VkFormat swapChainDepthFormat;
  VkExtent2D swapChainExtent;

  std::vector<VkFramebuffer> shadowFramebuffers;
  VkRenderPass shadowRenderPass;

  std::vector<VkFramebuffer> mappingsFramebuffers;
  VkRenderPass mappingsRenderPass;

  std::vector<VkFramebuffer> uvReflectionFramebuffers;
  VkRenderPass uvReflectionRenderPass;

  std::vector<VkFramebuffer> lightingFramebuffers;
  VkRenderPass lightingRenderPass;

  std::vector<VkFramebuffer> postProcessingFramebuffers;
  VkRenderPass postProcessingRenderPass;

  std::vector<Samplers> samplersVector;
  std::vector<Attachments> attachmentsVector;
  std::vector<VkImage> swapChainImages;
  std::vector<VkImageView> swapChainImageViews;

  Vk3dDevice &device;
  Vk3dAllocator &allocator;
  VkExtent2D windowExtent;

  VkSwapchainKHR swapChain;
  std::shared_ptr<Vk3dSwapChain> oldSwapChain;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  std::vector<VkFence> imagesInFlight;
  size_t currentFrame = 0;

  std::unique_ptr<Vk3dDescriptorSetLayout> gBufferSetLayout;
  std::unique_ptr<Vk3dDescriptorSetLayout> compositionSetLayout;
  std::unique_ptr<Vk3dDescriptorSetLayout> shadowSetLayout;
  std::unique_ptr<Vk3dDescriptorSetLayout> mappingsSetLayout;
  std::unique_ptr<Vk3dDescriptorSetLayout> uvReflectionSetLayout;
  std::unique_ptr<Vk3dDescriptorSetLayout> postProcessingSetLayout;
  std::unique_ptr<Vk3dDescriptorPool> globalPool;
  std::vector<std::unique_ptr<Vk3dBuffer>> gBufferUboBuffers;
  std::vector<std::unique_ptr<Vk3dBuffer>> compositionUboBuffers;
  std::vector<std::unique_ptr<Vk3dBuffer>> shadowUboBuffers;
  std::vector<std::unique_ptr<Vk3dBuffer>> mappingsUboBuffers;
  std::vector<std::unique_ptr<Vk3dBuffer>> uvReflectionUboBuffers;
  std::vector<std::unique_ptr<Vk3dBuffer>> postProcessingUboBuffers;
  std::vector<VkDescriptorSet> gBufferDescriptorSets;
  std::vector<VkDescriptorSet> compositionDescriptorSets;
  std::vector<VkDescriptorSet> shadowDescriptorSets;
  std::vector<VkDescriptorSet> mappingsDescriptorSets;
  std::vector<VkDescriptorSet> uvReflectionDescriptorSets;
  std::vector<VkDescriptorSet> postProcessingDescriptorSets;
};

}  // namespace vk3d
