#include "lve_swap_chain.hpp"

// std
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

namespace lve {
LveSwapChain::LveSwapChain(LveDevice &deviceRef, VkExtent2D extent)
    : device{deviceRef}, windowExtent{extent} {
    init();
}

LveSwapChain::LveSwapChain(LveDevice& deviceRef, VkExtent2D extent, std::shared_ptr<LveSwapChain> previous)
    : device{ deviceRef }, windowExtent{ extent }, oldSwapChain{ previous } {
    init();

    // clean up old swap chain since it's no longer needed
    oldSwapChain = nullptr;
}

void LveSwapChain::init() {
    createSwapChain();
    createSwapChainImageViews();
    createDeferredResources();
    createRenderPass();
    createFramebuffers();
    createSyncObjects();
}

LveSwapChain::~LveSwapChain() {
  for (auto imageView : swapChainImageViews) {
    vkDestroyImageView(device.device(), imageView, nullptr);
  }
  swapChainImageViews.clear();

  if (swapChain != nullptr) {
    vkDestroySwapchainKHR(device.device(), swapChain, nullptr);
    swapChain = nullptr;
  }

  destroyAttachment(&attachments.normal);
  destroyAttachment(&attachments.albedo);
  destroyAttachment(&attachments.depth);

  for (auto framebuffer : swapChainFramebuffers) {
    vkDestroyFramebuffer(device.device(), framebuffer, nullptr);
  }

  vkDestroyRenderPass(device.device(), renderPass, nullptr);

  // cleanup synchronization objects
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(device.device(), renderFinishedSemaphores[i], nullptr);
    vkDestroySemaphore(device.device(), imageAvailableSemaphores[i], nullptr);
    vkDestroyFence(device.device(), inFlightFences[i], nullptr);
  }
}

VkResult LveSwapChain::acquireNextImage(uint32_t *imageIndex) {
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

VkResult LveSwapChain::submitCommandBuffers(
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

void LveSwapChain::createSwapChain() {
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

void LveSwapChain::createSwapChainImageViews() {
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

void LveSwapChain::createRenderPass() {
  std::array<VkAttachmentDescription, 4> attachments {};

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
  // Depth attachment
  attachments[3].format = findDepthFormat();
  attachments[3].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[3].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  // Two subpasses
  std::array<VkSubpassDescription, 2> subpassDescriptions{};

  // First subpass: Fill G-Buffer components
  // ----------------------------------------------------------------------------------------

  VkAttachmentReference colorReferences[2];
  colorReferences[0] = { 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
  colorReferences[1] = { 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
  VkAttachmentReference depthReference = { 3, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

  subpassDescriptions[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpassDescriptions[0].colorAttachmentCount = 2;
  subpassDescriptions[0].pColorAttachments = colorReferences;
  subpassDescriptions[0].pDepthStencilAttachment = &depthReference;

  // Second subpass: Final composition (by using previous G-Buffer components)
  // ----------------------------------------------------------------------------------------

  VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

  VkAttachmentReference inputReferences[3];
  inputReferences[0] = { 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
  inputReferences[1] = { 2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
  inputReferences[2] = { 3, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

  subpassDescriptions[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpassDescriptions[1].colorAttachmentCount = 1;
  subpassDescriptions[1].pColorAttachments = &colorReference;
  subpassDescriptions[1].inputAttachmentCount = 3;
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

void LveSwapChain::createFramebuffers() {
  swapChainFramebuffers.resize(imageCount());
  for (size_t i = 0; i < imageCount(); i++) {
    std::array<VkImageView, 4> attachments = { swapChainImageViews[i], this->attachments.normal.view, this->attachments.albedo.view, this->attachments.depth.view };

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

void LveSwapChain::createDeferredResources() {
    VkExtent2D swapChainExtent = getSwapChainExtent();
    deferredResourcesFormat = VK_FORMAT_R32G32B32A32_SFLOAT;

    createAttachment(deferredResourcesFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &attachments.normal, swapChainExtent);
    createAttachment(deferredResourcesFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &attachments.albedo, swapChainExtent);
    createAttachment(findDepthFormat(), VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, &attachments.depth, swapChainExtent);
    
}

void LveSwapChain::createAttachment(VkFormat format, VkImageUsageFlags usage, FrameBufferAttachment* attachment, VkExtent2D swapChainExtent) {
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
    imageInfo.extent.width = swapChainExtent.width;
    imageInfo.extent.height = swapChainExtent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = attachment->format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = 0;

    device.createImageWithInfo(
        imageInfo,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        attachment->image,
        attachment->memory);

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

void LveSwapChain::createSyncObjects() {
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

VkSurfaceFormatKHR LveSwapChain::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats) {
  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

VkPresentModeKHR LveSwapChain::chooseSwapPresentMode(
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

VkExtent2D LveSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
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

VkFormat LveSwapChain::findDepthFormat() {
  return device.findSupportedFormat(
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void LveSwapChain::destroyAttachment(FrameBufferAttachment* attachment) {
    vkDestroyImageView(device.device(), attachment->view, nullptr);
    vkDestroyImage(device.device(), attachment->image, nullptr);
    vkFreeMemory(device.device(), attachment->memory, nullptr);
}

VkDescriptorImageInfo LveSwapChain::FrameBufferAttachment::descriptorInfo(VkSampler sampler, VkImageLayout imageLayout) {
    return VkDescriptorImageInfo{
            sampler,
            view,
            imageLayout
    };
}

}  // namespace lve
