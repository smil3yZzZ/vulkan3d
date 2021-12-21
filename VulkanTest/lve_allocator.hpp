#pragma once
#include "vk_mem_alloc.h"

#include "lve_device.hpp"

namespace lve {

    class LveAllocator {
    public:
        LveAllocator(LveDevice& deviceRef);
        ~LveAllocator();

        LveAllocator(const LveAllocator &) = delete;
        LveAllocator& operator=(const LveAllocator &) = delete;

        VmaAllocator getAllocator() { return allocator; }

        void createBuffer(VkDeviceSize size,
            VkBufferUsageFlags usage,
            VmaMemoryUsage memoryUsage,
            VkMemoryPropertyFlags properties,
            VkBuffer& buffer,
            VmaAllocation& constantBufferAllocation);
        void destroyBuffer(VkBuffer& buffer, VmaAllocation& constantBufferAllocation);

    private:
        LveDevice& device;
        VmaAllocator allocator;
        void createAllocator(VkPhysicalDevice physicalDevice, VkDevice device, VkInstance instance);
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    };

}