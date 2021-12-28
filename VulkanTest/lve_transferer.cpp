#include "lve_transferer.hpp"
#include <math.h>

#include <stdexcept>
#include <array>

#include <iostream>


namespace lve {

	LveTransferer::LveTransferer(LveDevice& device) : lveDevice{ device } {
		createCommandBuffers();
	}

	LveTransferer::~LveTransferer() {
		freeCommandBuffers();
	}

	void LveTransferer::createCommandBuffers() {
		commandBuffers.resize(MAX_TRANSFER_COMMAND_BUFFERS);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = lveDevice.getTransferCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw new std::runtime_error("failed to allocate command buffers");
		}
	}

	void LveTransferer::freeCommandBuffers() {
		std::cout << "Freeing command buffers" << std::endl;
		vkFreeCommandBuffers(lveDevice.device(), lveDevice.getGraphicsCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
		commandBuffers.clear();
	}

	VkCommandBuffer LveTransferer::beginBufferCopy() {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		if (vkBeginCommandBuffer(commandBuffers[0], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer");
		}

		std::cout << "Beginning buffer copy" << std::endl;
		return commandBuffers[0];
	}

	void LveTransferer::performBufferCopy(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;  // Optional
		copyRegion.dstOffset = 0;  // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	}

	void LveTransferer::setCopyBarrier(VkCommandBuffer commandBuffer) {
		VkMemoryBarrier copyBarrier{};
		copyBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		copyBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		copyBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
			0, 1, &copyBarrier, 0, nullptr, 0, nullptr);
	}
	
	void LveTransferer::endBufferCopy(VkCommandBuffer commandBuffer) {
		std::cout << "Ending command buffer" << std::endl;
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(lveDevice.transferQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		//free buffer after ending;
		//vkQueueWaitIdle(lveDevice.transferQueue());

		//vkFreeCommandBuffers(lveDevice.device(), lveDevice.getTransferCommandPool(), 1, &commandBuffer);
	}

}