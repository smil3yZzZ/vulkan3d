#include "vk_mem_alloc.h"

#include "lve_model.hpp"

// std
#include <cassert>
#include <cstring>

#include <iostream>

namespace lve {
	LveModel::LveModel(LveDevice& device, const LveModel::Builder& builder, LveAllocator& allocator) : lveDevice (device), lveAllocator (allocator){
		createVertexBuffers(builder.vertices);
		createIndexBuffers(builder.indices);
	}
	LveModel::~LveModel() {
		destroyVertexBuffers();
	}

	void LveModel::createVertexBuffers(const std::vector<Vertex>& vertices) {
		vertexCount = static_cast<uint32_t>(vertices.size());
		assert(vertexCount >= 3 && "Vertex count must be at least 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

		VmaAllocator allocator = lveAllocator.getAllocator();

		VkBuffer stagingBuffer;
		VmaAllocation stagingBufferAllocation;

		lveAllocator.createBuffer(bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_CPU_ONLY,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferAllocation
			);

		void* data;

		vmaMapMemory(allocator, stagingBufferAllocation, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vmaUnmapMemory(allocator, stagingBufferAllocation);

		lveAllocator.createBuffer(bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			vertexBuffer,
			vertexBufferAllocation
		);

		lveDevice.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		lveAllocator.destroyBuffer(stagingBuffer, stagingBufferAllocation);
	}

	void LveModel::createIndexBuffers(const std::vector<uint32_t>& indices) {
		indexCount = static_cast<uint32_t>(indices.size());
		hasIndexBuffer = indexCount > 0;

		if (!hasIndexBuffer) {
			return;
		}

		VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;

		VmaAllocator allocator = lveAllocator.getAllocator();

		VkBuffer stagingBuffer;
		VmaAllocation stagingBufferAllocation;

		lveAllocator.createBuffer(bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_CPU_ONLY,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferAllocation
		);

		void* data;

		vmaMapMemory(allocator, stagingBufferAllocation, &data);
		memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
		vmaUnmapMemory(allocator, stagingBufferAllocation);

		lveAllocator.createBuffer(bufferSize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			indexBuffer,
			indexBufferAllocation
		);

		lveDevice.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

		lveAllocator.destroyBuffer(stagingBuffer, stagingBufferAllocation);
	}

	void LveModel::destroyVertexBuffers() {
		lveAllocator.destroyBuffer(vertexBuffer, vertexBufferAllocation);
		if (hasIndexBuffer) {
			lveAllocator.destroyBuffer(indexBuffer, indexBufferAllocation);
		}
	}

	void LveModel::bind(VkCommandBuffer commandBuffer) {
		VkBuffer buffers[] = {vertexBuffer};
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

		if (hasIndexBuffer) {
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		}
	}

	void LveModel::draw(VkCommandBuffer commandBuffer) {
		if (hasIndexBuffer) {
			vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
		}
		else {
			vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
		}
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
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);
		return attributeDescriptions;
	}

}