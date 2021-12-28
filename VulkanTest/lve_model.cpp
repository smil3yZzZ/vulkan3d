#include "vk_mem_alloc.h"

#include "lve_model.hpp"

// std
#include <cassert>
#include <cstring>

#include <iostream>

namespace lve {
	LveModel::LveModel(LveDevice& device, LveModel::Builder builder, LveAllocator& allocator, LveTransferer& transferer) : lveDevice (device), lveAllocator (allocator), lveTransferer(transferer), builder(builder) {
		std::cout << "Creating model: " << builder.vertices.size() << std::endl;
		/*
		if (auto commandBuffer = lveTransferer.beginBufferCopy()) {
				LveModel::Buffer vertexStagingBuffer{};
				copyAndGetStagingVertexBuffers(commandBuffer, builder.vertices, vertexStagingBuffer);
				LveModel::Buffer indexStagingBuffer{};
				copyAndGetStagingIndexBuffers(commandBuffer, builder.indices, indexStagingBuffer);
				std::cout << "Index buffer created" << std::endl;
				lveTransferer.endBufferCopy(commandBuffer);
				std::cout << "Buffer copy ended" << std::endl;
				//lveAllocator.destroyBuffer(vertexStagingBuffer.data, vertexStagingBuffer.allocation);
				//lveAllocator.destroyBuffer(indexStagingBuffer.data, indexStagingBuffer.allocation);
		}
		else {
			throw std::runtime_error("failed to create buffers!");
		}
		*/
		//copyStagingVertexBuffers(builder.vertices);
		//copyStagingIndexBuffers(builder.indices);

	}
	LveModel::~LveModel() {
		destroyVertexBuffers();
	}

	void LveModel::createVertexStagingBuffers() {
		std::cout << builder.vertices.size() << std::endl;

		vertexCount = static_cast<uint32_t>(builder.vertices.size());
		assert(vertexCount >= 3 && "Vertex count must be at least 3");
		vertexBufferSize = sizeof(builder.vertices[0]) * vertexCount;

		VmaAllocator allocator = lveAllocator.getAllocator();

		//VkBuffer stagingBuffer;
		//VmaAllocation stagingBufferAllocation;

		std::cout << builder.vertices.size() << std::endl;
		std::cout << vertexCount << std::endl;

		lveAllocator.createBuffer(vertexBufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_CPU_ONLY,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			vertexStagingBuffer,
			vertexStagingBufferAllocation
			);

		void* data;



		vmaMapMemory(allocator, vertexStagingBufferAllocation, &data);
		memcpy(data, builder.vertices.data(), static_cast<size_t>(vertexBufferSize));
		vmaUnmapMemory(allocator, vertexStagingBufferAllocation);

		lveAllocator.createBuffer(vertexBufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			vertexBuffer,
			vertexBufferAllocation
		);

		//lveDevice.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
		
		/*
		if (auto commandBuffer = lveTransferer.beginBufferCopy()) {
			lveTransferer.performBufferCopy(commandBuffer, stagingBuffer, vertexBuffer, bufferSize);
			lveTransferer.endBufferCopy(commandBuffer);
		}
		else {
			throw std::runtime_error("failed to copy staging buffer!");
		}
		*/

		//lveTransferer.performBufferCopy(commandBuffer, vertexStagingBuffer, vertexBuffer, vertexBufferSize);

		//vertexStagingBuffer.data = stagingBuffer;
		//vertexStagingBuffer.allocation = stagingBufferAllocation;
		//lveAllocator.destroyBuffer(stagingBuffer, stagingBufferAllocation);
	}

	void LveModel::copyVertexStagingBuffers(VkCommandBuffer commandBuffer) {
		lveTransferer.performBufferCopy(commandBuffer, vertexStagingBuffer, vertexBuffer, vertexBufferSize);
	}

	void LveModel::createIndexStagingBuffers() {
		indexCount = static_cast<uint32_t>(builder.indices.size());
		hasIndexBuffer = indexCount > 0;

		if (!hasIndexBuffer) {
			return;
		}

		indexBufferSize = sizeof(builder.indices[0]) * indexCount;

		VmaAllocator allocator = lveAllocator.getAllocator();

		//VkBuffer stagingBuffer;
		//VmaAllocation stagingBufferAllocation;

		lveAllocator.createBuffer(indexBufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_CPU_ONLY,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			indexStagingBuffer,
			indexStagingBufferAllocation
		);

		void* data;

		vmaMapMemory(allocator, indexStagingBufferAllocation, &data);
		memcpy(data, builder.indices.data(), static_cast<size_t>(indexBufferSize));
		vmaUnmapMemory(allocator, indexStagingBufferAllocation);

		lveAllocator.createBuffer(indexBufferSize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			indexBuffer,
			indexBufferAllocation
		);

		//lveDevice.copyBuffer(stagingBuffer, indexBuffer, bufferSize);
		
		//lveTransferer.performBufferCopy(commandBuffer, indexStagingBuffer, indexBuffer, indexBufferSize);

		//indexStagingBuffer.data = stagingBuffer;
		//indexStagingBuffer.allocation = stagingBufferAllocation;

		//lveAllocator.destroyBuffer(stagingBuffer, stagingBufferAllocation);
	}

	void LveModel::copyIndexStagingBuffers(VkCommandBuffer commandBuffer) {
		lveTransferer.performBufferCopy(commandBuffer, indexStagingBuffer, indexBuffer, indexBufferSize);
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