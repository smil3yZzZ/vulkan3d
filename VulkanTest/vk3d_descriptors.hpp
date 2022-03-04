#pragma once

#include "vk3d_device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace vk3d {

    class Vk3dDescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(Vk3dDevice& lveDevice) : lveDevice{ lveDevice } {}

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<Vk3dDescriptorSetLayout> build() const;

        private:
            Vk3dDevice& lveDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        Vk3dDescriptorSetLayout(
            Vk3dDevice& lveDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~Vk3dDescriptorSetLayout();
        Vk3dDescriptorSetLayout(const Vk3dDescriptorSetLayout&) = delete;
        Vk3dDescriptorSetLayout& operator=(const Vk3dDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        Vk3dDevice& lveDevice;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class Vk3dDescriptorWriter;
    };

    class Vk3dDescriptorPool {
    public:
        class Builder {
        public:
            Builder(Vk3dDevice& lveDevice) : lveDevice{ lveDevice } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::unique_ptr<Vk3dDescriptorPool> build() const;

        private:
            Vk3dDevice& lveDevice;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        Vk3dDescriptorPool(
            Vk3dDevice& lveDevice,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~Vk3dDescriptorPool();
        Vk3dDescriptorPool(const Vk3dDescriptorPool&) = delete;
        Vk3dDescriptorPool& operator=(const Vk3dDescriptorPool&) = delete;

        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

    private:
        Vk3dDevice& lveDevice;
        VkDescriptorPool descriptorPool;

        friend class Vk3dDescriptorWriter;
    };

    class Vk3dDescriptorWriter {
    public:
        Vk3dDescriptorWriter(Vk3dDescriptorSetLayout& setLayout, Vk3dDescriptorPool& pool);

        Vk3dDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        Vk3dDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        Vk3dDescriptorSetLayout& setLayout;
        Vk3dDescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

}  // namespace vk3d