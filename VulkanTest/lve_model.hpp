#pragma once

#include "lve_device.hpp"
#include "lve_swap_chain.hpp"
#include "lve_allocator.hpp"
#include "lve_transferer.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <vector>

namespace lve {
	class LveModel {
		public:
			struct Vertex {
				glm::vec3 position;
				glm::vec4 color;
				static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
				static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
			};

			struct Builder {
				std::vector<Vertex> vertices{};
				std::vector<uint32_t> indices{};
			};

			struct Buffer {
				VkBuffer data;
				VmaAllocation allocation;
			};

			LveModel(LveDevice &device, LveModel::Builder builder, LveAllocator &allocator, LveTransferer &transferer);
			~LveModel();

			LveModel(const LveModel&) = delete;
			LveModel& operator=(const LveModel&) = delete;

			void bind(VkCommandBuffer commandBuffer);
			void draw(VkCommandBuffer commandBuffer);

			void createVertexStagingBuffers();
			void copyVertexStagingBuffers(VkCommandBuffer commandBuffer);

			void createIndexStagingBuffers();
			void copyIndexStagingBuffers(VkCommandBuffer commandBuffer);

			void destroyVertexBuffers();

		private:
			void createVertexBuffers();
			void createIndexBuffers();

			LveDevice& lveDevice;
			LveAllocator& lveAllocator;
			LveTransferer& lveTransferer;
			VkBuffer vertexBuffer;
			VmaAllocation vertexBufferAllocation;
			uint32_t vertexCount;
			VkDeviceSize vertexBufferSize;
			LveModel::Builder builder;

			bool hasIndexBuffer = false;
			VkBuffer indexBuffer;
			VmaAllocation indexBufferAllocation;
			uint32_t indexCount;
			VkDeviceSize indexBufferSize;

			VkBuffer vertexStagingBuffer;
			VmaAllocation vertexStagingBufferAllocation;
			VkBuffer indexStagingBuffer;
			VmaAllocation indexStagingBufferAllocation;
	};
}