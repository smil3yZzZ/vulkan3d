#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "lve_allocator.hpp"

#include <iostream>
#include <stdexcept>

namespace lve {

	LveAllocator::LveAllocator(LveDevice& deviceRef) : device{deviceRef}
	{
		createAllocator(device.getPhysicalDevice(), device.device(), device.getInstance());
	}

	LveAllocator::~LveAllocator() {
		vmaDestroyAllocator(allocator);
	}

	void LveAllocator::createAllocator(VkPhysicalDevice physicalDevice, VkDevice device, VkInstance instance) {
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
		allocatorInfo.physicalDevice = physicalDevice;
		allocatorInfo.device = device;
		allocatorInfo.instance = instance;
		
		vmaCreateAllocator(&allocatorInfo, &allocator);
	}

	void LveAllocator::createBuffer(VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer& buffer,
		VmaAllocation& constantBufferAllocation,
		VmaAllocator* allocator) {

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		allocInfo.requiredFlags = properties;

		vmaCreateBuffer(this->allocator, &bufferInfo, &allocInfo, &buffer, &constantBufferAllocation, nullptr);

		*allocator = this->allocator;
	}

	void LveAllocator::destroyBuffer(VkBuffer& buffer, VmaAllocation& constantBufferAllocation) {
		vmaDestroyBuffer(allocator, buffer, constantBufferAllocation);
	}

}