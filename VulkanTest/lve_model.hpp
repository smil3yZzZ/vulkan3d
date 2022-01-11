#pragma once

#include "lve_device.hpp"
#include "lve_buffer.hpp"
#include "lve_swap_chain.hpp"
#include "lve_allocator.hpp"


// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>

namespace lve {
	class LveModel {
		public:
			
			struct Vertex {
				glm::vec3 position{};
				glm::vec4 color{};
				glm::vec3 normal{};
				glm::vec2 uv{};
				static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
				static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

				bool operator==(const Vertex& other) const {
					return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
				}
			};

			struct Builder {
				std::vector<Vertex> vertices{};
				std::vector<uint32_t> indices{};

				void loadModel(const std::string &filepath);
			};

			LveModel(LveDevice &device, const LveModel::Builder &builder, LveAllocator &allocator);
			~LveModel();

			LveModel(const LveModel&) = delete;
			LveModel& operator=(const LveModel&) = delete;

			static std::unique_ptr<LveModel> createModelFromFile(LveDevice &device, const std::string &filepath, LveAllocator& allocator);

			void bind(VkCommandBuffer commandBuffer);
			void draw(VkCommandBuffer commandBuffer);

		private:
			void createVertexBuffers(const std::vector<Vertex>& vertices);
			void createIndexBuffers(const std::vector<uint32_t>& indices);
			void destroyVertexBuffers();

			LveDevice& lveDevice;
			LveAllocator& lveAllocator;
			std::unique_ptr<LveBuffer> vertexBuffer;
			uint32_t vertexCount;

			bool hasIndexBuffer = false;
			std::unique_ptr<LveBuffer> indexBuffer;
			uint32_t indexCount;
	};
}