#pragma once

#include "lve_device.hpp"
#include "lve_swap_chain.hpp"
#include "lve_allocator.hpp"

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
				glm::vec2 position;
				static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
				static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
			};

			LveModel(size_t vertexBufferIndex, LveDevice &device, const std::vector<Vertex> &vertices, LveAllocator &allocator);
			~LveModel();

			LveModel(const LveModel&) = delete;
			LveModel& operator=(const LveModel&) = delete;

			void bind(size_t vertexBufferIndex, VkCommandBuffer commandBuffer);
			void draw(VkCommandBuffer commandBuffer);

			void updateVertexBufferData(size_t vertexBufferIndex, const std::vector<Vertex>& vertices);

		private:
			void initVertexBuffers();
			void createVertexBuffer(size_t vertexBufferIndex, const std::vector<Vertex>& vertices);
			void destroyVertexBuffers();

			LveDevice& lveDevice;
			LveAllocator& lveAllocator;
			VkBuffer vertexBuffer;
			VkDeviceMemory vertexBufferMemory;
			std::vector<VkBuffer> vertexBuffers;
			std::vector<VmaAllocation> vertexBufferAllocations;
			uint32_t vertexCount;
	};
}