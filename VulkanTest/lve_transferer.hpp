#pragma once

#include "lve_device.hpp"

#include <cassert>
#include <memory>
#include <vector>

namespace lve {
	class LveTransferer {
	public:
		LveTransferer(LveDevice& device);
		~LveTransferer();

		LveTransferer(const LveTransferer&) = delete;
		LveTransferer& operator=(const LveTransferer&) = delete;

		VkCommandBuffer getCurrentCommandBuffer() const {
			return commandBuffers.back();
		};

		//VkCommandBuffer beginFrame();
		//void endFrame();
		VkCommandBuffer beginBufferCopy();
		void performBufferCopy(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void setCopyBarrier(VkCommandBuffer commandBuffer);
		void endBufferCopy(VkCommandBuffer commandBuffer);

	private:
		int MAX_TRANSFER_COMMAND_BUFFERS = 1;
		void createCommandBuffers();
		void freeCommandBuffers();

		LveDevice& lveDevice;

		std::vector<VkCommandBuffer> commandBuffers;
	};
}