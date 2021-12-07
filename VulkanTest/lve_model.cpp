#include "vk_mem_alloc.h"

#include "lve_model.hpp"

// std
#include <cassert>
#include <cstring>

#include <iostream>

namespace lve {
	LveModel::LveModel(size_t vertexBufferIndex, LveDevice& device, const std::vector<Vertex>& vertices, LveAllocator& allocator) : lveDevice (device), lveAllocator (allocator){
		initVertexBuffers();
		createVertexBuffer(vertexBufferIndex, vertices);
	}
	LveModel::~LveModel() {
		destroyVertexBuffers();
	}

	void LveModel::initVertexBuffers() {
		vertexBuffers.resize(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
		vertexBufferAllocations.resize(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
	}

	void LveModel::createVertexBuffer(size_t vertexBufferIndex, const std::vector<Vertex>& vertices) {
		vertexCount = static_cast<uint32_t>(vertices.size());
		assert(vertexCount >= 3 && "Vertex count must be at least 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

		VmaAllocator allocator;

		lveAllocator.createBuffer(bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			vertexBuffers[vertexBufferIndex],
			vertexBufferAllocations[vertexBufferIndex],
			&allocator
			);

		void* data;

		vmaMapMemory(allocator, vertexBufferAllocations[vertexBufferIndex], &data);
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vmaUnmapMemory(allocator, vertexBufferAllocations[vertexBufferIndex]);
	}

	void LveModel::destroyVertexBuffers() {
		for (int i = 0; i < vertexBuffers.size(); i++) {
			lveAllocator.destroyBuffer(vertexBuffers[i], vertexBufferAllocations[i]);
		}
	}

	void LveModel::updateVertexBufferData(size_t vertexBufferIndex, const std::vector<Vertex>& vertices) {
		lveAllocator.destroyBuffer(vertexBuffers[vertexBufferIndex], vertexBufferAllocations[vertexBufferIndex]);
		createVertexBuffer(vertexBufferIndex, vertices);
	}

	void LveModel::bind(size_t vertexBufferIndex, VkCommandBuffer commandBuffer) {
		VkBuffer buffers[] = {vertexBuffers[vertexBufferIndex]};
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	}

	void LveModel::draw(VkCommandBuffer commandBuffer) {
		vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
	}

	std::vector<VkVertexInputBindingDescription> LveModel::Vertex::getBindingDescriptions() {
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;
	}

	std::vector<VkVertexInputAttributeDescription> LveModel::Vertex::getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);
		return attributeDescriptions;
	}

}