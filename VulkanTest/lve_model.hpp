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
				glm::vec3 position;
				glm::vec4 color;
				static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
				static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
			};

			LveModel(LveDevice &device, const std::vector<Vertex> &vertices, LveAllocator &allocator);
			~LveModel();

			LveModel(const LveModel&) = delete;
			LveModel& operator=(const LveModel&) = delete;

			void bind(VkCommandBuffer commandBuffer);
			void draw(VkCommandBuffer commandBuffer);

		private:
			void createVertexBuffer(const std::vector<Vertex>& vertices);
			void destroyVertexBuffer();

			LveDevice& lveDevice;
			LveAllocator& lveAllocator;
			VkBuffer vertexBuffer;
			VmaAllocation vertexBufferAllocation;
			uint32_t vertexCount;
	};
}