#pragma once
#include "vk_mem_alloc.h"

#include "vk3d_device.hpp"

namespace vk3d {

    class Vk3dAllocator {
    public:
        Vk3dAllocator(Vk3dDevice& deviceRef);
        ~Vk3dAllocator();

        Vk3dAllocator(const Vk3dAllocator &) = delete;
        Vk3dAllocator& operator=(const Vk3dAllocator &) = delete;

        VmaAllocator getAllocator() { return allocator; }

        void createBuffer(VkDeviceSize size,
            VkBufferUsageFlags usage,
            VmaMemoryUsage memoryUsage,
            VkMemoryPropertyFlags properties,
            VkBuffer& buffer,
            VmaAllocation& constantBufferAllocation);
        void destroyBuffer(VkBuffer& buffer, VmaAllocation& constantBufferAllocation);

    private:
        Vk3dDevice& device;
        VmaAllocator allocator;
        void createAllocator(VkPhysicalDevice physicalDevice, VkDevice device, VkInstance instance);
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    };

}