#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "vk3d_allocator.hpp"

#include <iostream>
#include <stdexcept>

namespace vk3d {

	Vk3dAllocator::Vk3dAllocator(Vk3dDevice& deviceRef) : device{deviceRef}
	{
		createAllocator(device.getPhysicalDevice(), device.device(), device.getInstance());
	}

	Vk3dAllocator::~Vk3dAllocator() {
		vmaDestroyAllocator(allocator);
	}

	void Vk3dAllocator::createAllocator(VkPhysicalDevice physicalDevice, VkDevice device, VkInstance instance) {
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
		allocatorInfo.physicalDevice = physicalDevice;
		allocatorInfo.device = device;
		allocatorInfo.instance = instance;
		
		vmaCreateAllocator(&allocatorInfo, &allocator);
	}

	void Vk3dAllocator::createBuffer(VkDeviceSize size,
		VkBufferUsageFlags bufferUsage,
		VmaMemoryUsage memoryUsage,
		VkMemoryPropertyFlags properties,
		VkBuffer& buffer,
		VmaAllocation& constantBufferAllocation) {

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = bufferUsage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = memoryUsage;
		allocInfo.requiredFlags = properties;

		vmaCreateBuffer(this->allocator, &bufferInfo, &allocInfo, &buffer, &constantBufferAllocation, nullptr);
	}

	void Vk3dAllocator::destroyBuffer(VkBuffer& buffer, VmaAllocation& constantBufferAllocation) {
		vmaDestroyBuffer(allocator, buffer, constantBufferAllocation);
	}

	void Vk3dAllocator::createImage(VkImageCreateInfo* imageInfo,
		VmaMemoryUsage memoryUsage,
		VkMemoryPropertyFlags properties,
		VkImage& image,
		VmaAllocation& constantImageAllocation) {

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = memoryUsage;
		allocInfo.requiredFlags = properties;

		vmaCreateImage(this->allocator, imageInfo, &allocInfo, &image, &constantImageAllocation, nullptr);
	}

	void Vk3dAllocator::destroyImage(VkImage& image, VmaAllocation& constantImageAllocation) {
		vmaDestroyImage(allocator, image, constantImageAllocation);
	}

}