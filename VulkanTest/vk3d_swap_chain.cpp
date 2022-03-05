#include "vk3d_swap_chain.hpp"

// std
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

namespace vk3d {
Vk3dSwapChain::Vk3dSwapChain(Vk3dDevice &deviceRef, Vk3dAllocator &allocatorRef, VkExtent2D extent)
    : device{ deviceRef }, allocator{ allocatorRef }, windowExtent{ extent }  {
    init();
}

Vk3dSwapChain::Vk3dSwapChain(Vk3dDevice& deviceRef, Vk3dAllocator& allocatorRef, VkExtent2D extent, std::shared_ptr<Vk3dSwapChain> previous)
    : device{ deviceRef }, allocator{ allocatorRef }, windowExtent{ extent }, oldSwapChain{ previous } {
    init();

    // clean up old swap chain since it's no longer needed
    oldSwapChain = nullptr;
}

void Vk3dSwapChain::init() {
    createSwapChain();
    createSwapChainImageViews();
    createShadowSampler();
    createDeferredResources();
    createShadowRenderPass();
    createCompositionRenderPass();
    createShadowFramebuffers();
    createCompositionFramebuffers();
    createSyncObjects();
    createDescriptorPool();
    createUniformBuffers();
}

Vk3dSwapChain::~Vk3dSwapChain() {
  for (auto imageView : swapChainImageViews) {
    vkDestroyImageView(device.device(), imageView, nullptr);
  }
  swapChainImageViews.clear();

  if (swapChain != nullptr) {
    vkDestroySwapchainKHR(device.device(), swapChain, nullptr);
    swapChain = nullptr;
  }

  for (auto& attachments : attachmentsVector) {
      destroyAttachment(&attachments.normal);
      destroyAttachment(&attachments.albedo);
      destroyAttachment(&attachments.depth);
      destroyAttachment(&attachments.lightSpace);
  }
  attachmentsVector.clear();

  for (auto& samplers : samplersVector) {
      destroySampler(&samplers.shadowDepth);
  }
  samplersVector.clear();
  

  for (auto framebuffer : swapChainFramebuffers) {
    vkDestroyFramebuffer(device.device(), framebuffer, nullptr);
  }

  for (auto framebuffer : shadowFramebuffers) {
      vkDestroyFramebuffer(device.device(), framebuffer, nullptr);
  }

  vkDestroyRenderPass(device.device(), renderPass, nullptr);
  vkDestroyRenderPass(device.device(), shadowRenderPass, nullptr);

  // cleanup synchronization objects
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(device.device(), renderFinishedSemaphores[i], nullptr);
    vkDestroySemaphore(device.device(), imageAvailableSemaphores[i], nullptr);
    vkDestroyFence(device.device(), inFlightFences[i], nullptr);
  }
}

VkResult Vk3dSwapChain::acquireNextImage(uint32_t *imageIndex) {
  vkWaitForFences(
      device.device(),
      1,
      &inFlightFences[currentFrame],
      VK_TRUE,
      std::numeric_limits<uint64_t>::max());

  VkResult result = vkAcquireNextImageKHR(
      device.device(),
      swapChain,
      std::numeric_limits<uint64_t>::max(),
      imageAvailableSemaphores[currentFrame],  // must be a not signaled semaphore
      VK_NULL_HANDLE,
      imageIndex);

  return result;
}

VkResult Vk3dSwapChain::submitCommandBuffers(
    const VkCommandBuffer *buffers, uint32_t *imageIndex) {
  if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
    vkWaitForFences(device.device(), 1, &imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
  }
  imagesInFlight[*imageIndex] = inFlightFences[currentFrame];

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = buffers;

  VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  vkResetFences(device.device(), 1, &inFlightFences[currentFrame]);

  if (vkQueueSubmit(device.graphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to submit draw command buffer!");
  }

  VkPresentInfoKHR presentInfo = {};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {swapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;

  presentInfo.pImageIndices = imageIndex;

  auto result = vkQueuePresentKHR(device.presentQueue(), &presentInfo);

  currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

  return result;
}

void Vk3dSwapChain::createSwapChain() {
  SwapChainSupportDetails swapChainSupport = device.getSwapChainSupport();

  VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 &&
      imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = device.surface();

  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices = device.findPhysicalQueueFamilies();
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

  if (indices.graphicsFamily != indices.presentFamily) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;      // Optional
    createInfo.pQueueFamilyIndices = nullptr;  // Optional
  }

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  createInfo.oldSwapchain = oldSwapChain == nullptr ? VK_NULL_HANDLE : oldSwapChain->swapChain;

  if (vkCreateSwapchainKHR(device.device(), &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
    throw std::runtime_error("failed to create swap chain!");
  }

  // we only specified a minimum number of images in the swap chain, so the implementation is
  // allowed to create a swap chain with more. That's why we'll first query the final number of
  // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
  // retrieve the handles.
  vkGetSwapchainImagesKHR(device.device(), swapChain, &imageCount, nullptr);
  swapChainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(device.device(), swapChain, &imageCount, swapChainImages.data());

  swapChainImageFormat = surfaceFormat.format;
  swapChainExtent = extent;
}

void Vk3dSwapChain::createSwapChainImageViews() {
  swapChainImageViews.resize(swapChainImages.size());
  for (size_t i = 0; i < swapChainImages.size(); i++) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = swapChainImages[i];
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = swapChainImageFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device.device(), &viewInfo, nullptr, &swapChainImageViews[i]) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to create texture image view!");
    }
  }
}

void Vk3dSwapChain::createCompositionRenderPass() {
  std::array<VkAttachmentDescription, 5> attachments {};

  // Color attachment (swap chain)
  attachments[0].format = getSwapChainImageFormat();
  attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  // Deferred attachments
  // Normals
  attachments[1].format = deferredResourcesFormat;
  attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[1].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  // Albedo
  attachments[2].format = deferredResourcesFormat;
  attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[2].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  // Light Space position
  attachments[3].format = deferredResourcesFormat;
  attachments[3].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[3].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  // Depth attachment
  attachments[4].format = findDepthFormat();
  attachments[4].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[4].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[4].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[4].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[4].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[4].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[4].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  // Two subpasses
  std::array<VkSubpassDescription, 2> subpassDescriptions{};

  // First subpass: Fill G-Buffer components
  // ----------------------------------------------------------------------------------------

  VkAttachmentReference colorReferences[3];
  colorReferences[0] = { 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
  colorReferences[1] = { 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
  colorReferences[2] = { 3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
  VkAttachmentReference depthReference = { 4, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

  subpassDescriptions[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpassDescriptions[0].colorAttachmentCount = 3;
  subpassDescriptions[0].pColorAttachments = colorReferences;
  subpassDescriptions[0].pDepthStencilAttachment = &depthReference;

  // Second subpass: Final composition (by using previous G-Buffer components)
  // ----------------------------------------------------------------------------------------

  VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

  VkAttachmentReference inputReferences[4];
  inputReferences[0] = { 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
  inputReferences[1] = { 2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
  inputReferences[2] = { 3, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
  inputReferences[3] = { 4, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

  subpassDescriptions[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpassDescriptions[1].colorAttachmentCount = 1;
  subpassDescriptions[1].pColorAttachments = &colorReference;
  subpassDescriptions[1].inputAttachmentCount = 4;
  subpassDescriptions[1].pInputAttachments = inputReferences;


  // Subpass dependencies for layout transitions
  std::array<VkSubpassDependency, 3> dependencies;

  dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[0].dstSubpass = 0;
  dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependencies[0].srcAccessMask = 0;
  dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  // This dependency transitions the input attachment from color attachment to shader read
  dependencies[1].srcSubpass = 0;
  dependencies[1].dstSubpass = 1;
  dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  dependencies[2].srcSubpass = 1;
  dependencies[2].dstSubpass = VK_SUBPASS_EXTERNAL;
  dependencies[2].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  dependencies[2].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependencies[2].srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
  dependencies[2].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  VkRenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = subpassDescriptions.size();
  renderPassInfo.pSubpasses = subpassDescriptions.data();
  renderPassInfo.dependencyCount = dependencies.size();
  renderPassInfo.pDependencies = dependencies.data();

  if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
    throw std::runtime_error("failed to create render pass!");
  }
}

void Vk3dSwapChain::createCompositionFramebuffers() {
  swapChainFramebuffers.resize(imageCount());
  for (size_t i = 0; i < imageCount(); i++) {
    std::array<VkImageView, 5> attachments = { swapChainImageViews[i], this->attachmentsVector[i].normal.view, this->attachmentsVector[i].albedo.view, this->attachmentsVector[i].lightSpace.view, this->attachmentsVector[i].depth.view };

    VkExtent2D swapChainExtent = getSwapChainExtent();
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = swapChainExtent.width;
    framebufferInfo.height = swapChainExtent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(
            device.device(),
            &framebufferInfo,
            nullptr,
            &swapChainFramebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create framebuffer!");
    }
  }
}

void Vk3dSwapChain::createDeferredResources() {
    VkExtent2D swapChainExtent = getSwapChainExtent();
    deferredResourcesFormat = VK_FORMAT_R32G32B32A32_SFLOAT;

    attachmentsVector.resize(imageCount());

    for (auto& attachments : attachmentsVector) {
        createAttachment(deferredResourcesFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, &attachments.normal, swapChainExtent);
        createAttachment(deferredResourcesFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, &attachments.albedo, swapChainExtent);
        createAttachment(deferredResourcesFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, &attachments.lightSpace, swapChainExtent);
        createAttachment(findDepthFormat(), VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, &attachments.depth, swapChainExtent);
    }   
}

void Vk3dSwapChain::createSampler(VkFormat format, VkImageUsageFlags usage, Sampler* sampler, VkExtent2D extent) {
    VkFilter shadowMapFilter = formatIsFilterable(device.getPhysicalDevice(), format, VK_IMAGE_TILING_OPTIMAL) ?
        DEFAULT_SHADOWMAP_FILTER :
        VK_FILTER_NEAREST;
    VkSamplerCreateInfo samplerCreateInfo{};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = shadowMapFilter;
    samplerCreateInfo.minFilter = shadowMapFilter;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeU;
    samplerCreateInfo.addressModeW = samplerCreateInfo.addressModeU;
    samplerCreateInfo.mipLodBias = 0.0f;
    samplerCreateInfo.anisotropyEnable = false;
    samplerCreateInfo.minLod = 0.0f;
    samplerCreateInfo.maxLod = 1.0f;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    if (vkCreateSampler(device.device(), &samplerCreateInfo, nullptr, &sampler->sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shadow depth sampler!");
    }
    createAttachment(format, usage, &sampler->attachment, extent);
}

void Vk3dSwapChain::createAttachment(VkFormat format, VkImageUsageFlags usage, FrameBufferAttachment* attachment, VkExtent2D extent) {
    attachment->format = format;

    VkImageAspectFlags imageAspectFlags;

    if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        imageAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    else {
        imageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = extent.width;
    imageInfo.extent.height = extent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = attachment->format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = 0;

    allocator.createImage(&imageInfo, VMA_MEMORY_USAGE_GPU_ONLY, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, attachment->image, attachment->memory);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = attachment->image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = attachment->format;
    viewInfo.subresourceRange.aspectMask = imageAspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device.device(), &viewInfo, nullptr, &attachment->view) != VK_SUCCESS) {
        throw std::runtime_error("failed to create position image view!");
    }
}

void Vk3dSwapChain::createSyncObjects() {
  imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
  imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

  VkSemaphoreCreateInfo semaphoreInfo = {};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo = {};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    if (vkCreateSemaphore(device.device(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) !=
            VK_SUCCESS ||
        vkCreateSemaphore(device.device(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) !=
            VK_SUCCESS ||
        vkCreateFence(device.device(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
  }
}

VkSurfaceFormatKHR Vk3dSwapChain::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats) {
  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

VkPresentModeKHR Vk3dSwapChain::chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> &availablePresentModes) {
  for (const auto &availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      std::cout << "Present mode: Mailbox" << std::endl;
      return availablePresentMode;
    }
  }

  // for (const auto &availablePresentMode : availablePresentModes) {
  //   if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
  //     std::cout << "Present mode: Immediate" << std::endl;
  //     return availablePresentMode;
  //   }
  // }

  std::cout << "Present mode: V-Sync" << std::endl;
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Vk3dSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    VkExtent2D actualExtent = windowExtent;
    actualExtent.width = std::max(
        capabilities.minImageExtent.width,
        std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(
        capabilities.minImageExtent.height,
        std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
  }
}

VkFormat Vk3dSwapChain::findDepthFormat() {
  return device.findSupportedFormat(
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void Vk3dSwapChain::destroyAttachment(FrameBufferAttachment* attachment) {
    vkDestroyImageView(device.device(), attachment->view, nullptr);
    allocator.destroyImage(attachment->image, attachment->memory);
}

VkDescriptorImageInfo Vk3dSwapChain::FrameBufferAttachment::descriptorInfo(VkSampler sampler, VkImageLayout imageLayout) {
    return VkDescriptorImageInfo{
            sampler,
            view,
            imageLayout
    };
}

void Vk3dSwapChain::createShadowSampler() {
    VkExtent2D shadowMapExtent = getShadowMapExtent();

    samplersVector.resize(imageCount());

    for (auto& samplers : samplersVector) {
        createSampler(findDepthFormat(), VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, &samplers.shadowDepth, shadowMapExtent);
    }
}

void Vk3dSwapChain::createShadowRenderPass() {
    std::array<VkAttachmentDescription, 1> attachments{};

    // Depth attachment (shadow)
    attachments[0].format = findDepthFormat();
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    attachments[0].flags = 0;

    // One subpass
    std::array<VkSubpassDescription, 1> subpassDescriptions{};

    VkAttachmentReference depthReference = { 0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

    subpassDescriptions[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescriptions[0].colorAttachmentCount = 0;
    subpassDescriptions[0].pColorAttachments = nullptr;
    subpassDescriptions[0].pDepthStencilAttachment = &depthReference;

    // Subpass dependencies for layout transitions
    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = 0;
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = subpassDescriptions.size();
    renderPassInfo.pSubpasses = subpassDescriptions.data();
    renderPassInfo.dependencyCount = dependencies.size();
    renderPassInfo.pDependencies = dependencies.data();

    if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &shadowRenderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void Vk3dSwapChain::createShadowFramebuffers() {
    shadowFramebuffers.resize(imageCount());
    VkExtent2D shadowMapExtent = getShadowMapExtent();
    for (size_t i = 0; i < imageCount(); i++) {
        std::array<VkImageView, 1> attachments = {samplersVector[i].shadowDepth.attachment.view};

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = shadowRenderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = shadowMapExtent.width;
        framebufferInfo.height = shadowMapExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(
            device.device(),
            &framebufferInfo,
            nullptr,
            &shadowFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void Vk3dSwapChain::destroySampler(Sampler* sampler) {
    destroyAttachment(&sampler->attachment);
    vkDestroySampler(device.device(), sampler->sampler, nullptr);
}

void Vk3dSwapChain::createDescriptorPool() {
    globalPool = Vk3dDescriptorPool::Builder(device)
        .setMaxSets(2 * imageCount())
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2 * imageCount())
        .addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 4 * imageCount())
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageCount())
        .build();
}

void Vk3dSwapChain::createUniformBuffers() {
    gBufferUboBuffers.clear();
    compositionUboBuffers.clear();
    gBufferUboBuffers.resize(imageCount());
    compositionUboBuffers.resize(imageCount());

    for (int i = 0; i < imageCount(); i++) {
        gBufferUboBuffers[i] = std::make_unique<Vk3dBuffer>(
            device,
            sizeof(GBufferUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VMA_MEMORY_USAGE_CPU_TO_GPU,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            allocator
            );
        gBufferUboBuffers[i]->map();
        compositionUboBuffers[i] = std::make_unique<Vk3dBuffer>(
            device,
            sizeof(CompositionUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VMA_MEMORY_USAGE_CPU_TO_GPU,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            allocator
            );
        compositionUboBuffers[i]->map();
    }

    gBufferSetLayout = Vk3dDescriptorSetLayout::Builder(device)
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
        .build();

    gBufferDescriptorSets.clear();
    gBufferDescriptorSets.resize(imageCount());
    for (int i = 0; i < gBufferDescriptorSets.size(); i++) {
        auto bufferInfo = gBufferUboBuffers[i]->descriptorInfo();
        Vk3dDescriptorWriter(*gBufferSetLayout, *globalPool)
            .writeBuffer(0, &bufferInfo)
            .build(gBufferDescriptorSets[i]);
    }

    compositionSetLayout = Vk3dDescriptorSetLayout::Builder(device)
        .addBinding(0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
        .addBinding(1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
        .addBinding(2, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
        .addBinding(3, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
        .addBinding(4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
        .addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();

    compositionDescriptorSets.clear();
    compositionDescriptorSets.resize(imageCount());

    for (int i = 0; i < compositionDescriptorSets.size(); i++) {
        auto normalInfo = attachmentsVector[i].normal.descriptorInfo();
        auto albedoInfo = attachmentsVector[i].albedo.descriptorInfo();
        auto lightSpaceInfo = attachmentsVector[i].lightSpace.descriptorInfo();
        auto depthInfo = attachmentsVector[i].depth.descriptorInfo();
        auto bufferInfo = compositionUboBuffers[i]->descriptorInfo();
        auto shadowDepthInfo = samplersVector[i].shadowDepth.attachment.descriptorInfo(samplersVector[i].shadowDepth.sampler);
        Vk3dDescriptorWriter(*compositionSetLayout, *globalPool)
            .writeImage(0, &normalInfo)
            .writeImage(1, &albedoInfo)
            .writeImage(2, &lightSpaceInfo)
            .writeImage(3, &depthInfo)
            .writeBuffer(4, &bufferInfo)
            .writeImage(5, &shadowDepthInfo)
            .build(compositionDescriptorSets[i]);
    }
}

void Vk3dSwapChain::updateCurrentGBufferUbo(void* data, int currentImageIndex) {
    gBufferUboBuffers[currentImageIndex]->writeToBuffer(data);
    gBufferUboBuffers[currentImageIndex]->flush();
}
void Vk3dSwapChain::updateCurrentCompositionUbo(void* data, int currentImageIndex) {
    compositionUboBuffers[currentImageIndex]->writeToBuffer(data);
    compositionUboBuffers[currentImageIndex]->flush();
}

// Returns if a given format support LINEAR filtering
VkBool32 Vk3dSwapChain::formatIsFilterable(VkPhysicalDevice physicalDevice, VkFormat format, VkImageTiling tiling)
{
    VkFormatProperties formatProps;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);

    if (tiling == VK_IMAGE_TILING_OPTIMAL)
        return formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

    if (tiling == VK_IMAGE_TILING_LINEAR)
        return formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

    return false;
}

}  // namespace vk3d
