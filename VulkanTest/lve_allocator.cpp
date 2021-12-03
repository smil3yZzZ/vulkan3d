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

	/*
	size: bufferSize,
			usage: VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			properties: VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			vertexBuffer?: vertexBuffers[vertexBufferIndex],
			vertexBufferMemory?: vertexBufferMemoryObjects[vertexBufferIndex]
	*/

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

		//allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
		vmaCreateBuffer(this->allocator, &bufferInfo, &allocInfo, &buffer, &constantBufferAllocation, nullptr);
		std::cout << "Buffer created!" << std::endl;

		*allocator = this->allocator;
	}

	void LveAllocator::destroyBuffer(VkBuffer& buffer, VmaAllocation& constantBufferAllocation) {
		vmaDestroyBuffer(allocator, buffer, constantBufferAllocation);
	}

	uint32_t LveAllocator::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(device.getPhysicalDevice(), &memProperties);
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) &&
				(memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

}