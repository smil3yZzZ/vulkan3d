#pragma once

#include "vk3d_device.hpp"
#include "vk3d_buffer.hpp"
#include "vk3d_swap_chain.hpp"
#include "vk3d_allocator.hpp"


// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>

namespace vk3d {
	class Vk3dModel {
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

			Vk3dModel(Vk3dDevice &device, const Vk3dModel::Builder &builder, Vk3dAllocator &allocator);
			~Vk3dModel();

			Vk3dModel(const Vk3dModel&) = delete;
			Vk3dModel& operator=(const Vk3dModel&) = delete;

			static std::unique_ptr<Vk3dModel> createModelFromFile(Vk3dDevice &device, const std::string &filepath, Vk3dAllocator& allocator);

			void bind(VkCommandBuffer commandBuffer);
			void draw(VkCommandBuffer commandBuffer);

		private:
			void createVertexBuffers(const std::vector<Vertex>& vertices);
			void createIndexBuffers(const std::vector<uint32_t>& indices);
			void destroyVertexBuffers();

			Vk3dDevice& lveDevice;
			Vk3dAllocator& lveAllocator;
			std::unique_ptr<Vk3dBuffer> vertexBuffer;
			uint32_t vertexCount;

			bool hasIndexBuffer = false;
			std::unique_ptr<Vk3dBuffer> indexBuffer;
			uint32_t indexCount;
	};
}