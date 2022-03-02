#include "shadow_render_system.hpp"
#include <math.h>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>

#include <iostream>


namespace lve {

	struct ShadowPushConstantData {
		glm::mat4 modelMatrix{ 1.f };
		glm::mat4 lightProjView{ 1.f };
	};

	ShadowRenderSystem::ShadowRenderSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout shadowSetLayout) : lveDevice{ device } {
		createShadowPipelineLayout(shadowSetLayout);
		createShadowPipeline(renderPass);
	}

	ShadowRenderSystem::~ShadowRenderSystem() {
		vkDestroyPipelineLayout(lveDevice.device(), shadowPipelineLayout, nullptr);
	}

	void ShadowRenderSystem::createShadowPipelineLayout(VkDescriptorSetLayout shadowSetLayout) {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(ShadowPushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ shadowSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &shadowPipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void ShadowRenderSystem::createShadowPipeline(VkRenderPass renderPass) {
		assert(shadowPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
		PipelineConfigInfo pipelineConfig{};
		pipelineConfig.attachmentCount = 1;
		pipelineConfig.hasVertexBufferBound = true;
		LvePipeline::shadowPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.subpass = 0;
		pipelineConfig.pipelineLayout = shadowPipelineLayout;
		lveShadowPipeline = std::make_unique<LvePipeline>(
			lveDevice,
			"shaders/shadow_shader.vert.spv",
			"shaders/shadow_shader.frag.spv",
			pipelineConfig
			);
	}

	void ShadowRenderSystem::renderGameObjects(FrameInfo& frameInfo, glm::mat4 lightProjView) {
		//Set depth bias in order to avoid artifacts
		vkCmdSetDepthBias(
			frameInfo.commandBuffer,
			depthBiasConstant,
			0.0f,
			depthBiasSlope);

		lveShadowPipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			shadowPipelineLayout,
			0,
			1,
			&frameInfo.shadowDescriptorSet,
			0,
			nullptr);

		for (auto& kv : frameInfo.gameObjects) {
			auto& obj = kv.second;

			ShadowPushConstantData push{};

			push.modelMatrix = obj.transform.mat4();
			push.lightProjView = lightProjView;

			vkCmdPushConstants(
				frameInfo.commandBuffer,
				shadowPipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT,
				0,
				sizeof(ShadowPushConstantData),
				&push
			);

			obj.model->bind(frameInfo.commandBuffer);
			obj.model->draw(frameInfo.commandBuffer);
		}
	}

	void ShadowRenderSystem::executeRenderPassCommands(FrameInfo& frameInfo, glm::mat4 projectionMatrix, glm::mat4 viewMatrix, LveRenderer* lveRenderer) {
		for (uint32_t faceIndex = 0; faceIndex < LveRenderer::NUM_CUBE_FACES; faceIndex++) {
			lveRenderer->beginShadowRenderPass(frameInfo.commandBuffer);

			switch (faceIndex)
			{
			case 0: // POSITIVE_X
				viewMatrix = glm::rotate(viewMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				viewMatrix = glm::rotate(viewMatrix, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				break;
			case 1:	// NEGATIVE_X
				viewMatrix = glm::rotate(viewMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				viewMatrix = glm::rotate(viewMatrix, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				break;
			case 2:	// POSITIVE_Y
				viewMatrix = glm::rotate(viewMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				break;
			case 3:	// NEGATIVE_Y
				viewMatrix = glm::rotate(viewMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				break;
			case 4:	// POSITIVE_Z
				viewMatrix = glm::rotate(viewMatrix, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				break;
			case 5:	// NEGATIVE_Z
				viewMatrix = glm::rotate(viewMatrix, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				break;
			}

			renderGameObjects(frameInfo, projectionMatrix * viewMatrix);

			lveRenderer->endShadowRenderPass(frameInfo.commandBuffer);

			copyImageToCube(frameInfo, faceIndex, lveRenderer);
		}
	}

	void ShadowRenderSystem::copyImageToCube(FrameInfo& frameInfo, uint32_t faceIndex, LveRenderer* lveRenderer) {
		
		// Make sure color writes to the framebuffer are finished before using it as transfer source
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 1;

		VkImageMemoryBarrier imageMemoryBarrier;
		imageMemoryBarrier = {};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageMemoryBarrier.image = lveRenderer->getAttachments().shadowColor.image;
		imageMemoryBarrier.subresourceRange = subresourceRange;

		vkCmdPipelineBarrier(
			frameInfo.commandBuffer,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);

		// Change image layout of one cubemap face to transfer destination

		VkImageSubresourceRange cubeFaceSubresourceRange = {};
		cubeFaceSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		cubeFaceSubresourceRange.baseMipLevel = 0;
		cubeFaceSubresourceRange.levelCount = 1;
		cubeFaceSubresourceRange.baseArrayLayer = faceIndex;
		cubeFaceSubresourceRange.layerCount = 1;

		imageMemoryBarrier = {};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageMemoryBarrier.image = lveRenderer->getSamplers().shadowCubeMap.attachment.image;
		imageMemoryBarrier.subresourceRange = cubeFaceSubresourceRange;

		vkCmdPipelineBarrier(
			frameInfo.commandBuffer,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);

		// Copy region for transfer from framebuffer to cube face
		VkImageCopy copyRegion = {};

		copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.srcSubresource.baseArrayLayer = 0;
		copyRegion.srcSubresource.mipLevel = 0;
		copyRegion.srcSubresource.layerCount = 1;
		copyRegion.srcOffset = { 0, 0, 0 };

		copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.dstSubresource.baseArrayLayer = faceIndex;
		copyRegion.dstSubresource.mipLevel = 0;
		copyRegion.dstSubresource.layerCount = 1;
		copyRegion.dstOffset = { 0, 0, 0 };

		copyRegion.extent.width = lveRenderer->getShadowMapExtent().width;
		copyRegion.extent.height = lveRenderer->getShadowMapExtent().height;
		copyRegion.extent.depth = 1;

		// Put image copy into command buffer

		vkCmdCopyImage(
			frameInfo.commandBuffer,
			lveRenderer->getAttachments().shadowColor.image,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			lveRenderer->getSamplers().shadowCubeMap.attachment.image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&copyRegion);

		// Transform framebuffer color attachment back

		imageMemoryBarrier = {};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		imageMemoryBarrier.image = lveRenderer->getAttachments().shadowColor.image;
		imageMemoryBarrier.subresourceRange = subresourceRange;

		vkCmdPipelineBarrier(
			frameInfo.commandBuffer,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);

		// Change image layout of copied face to shader read

		imageMemoryBarrier = {};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageMemoryBarrier.image = lveRenderer->getSamplers().shadowCubeMap.attachment.image;
		imageMemoryBarrier.subresourceRange = cubeFaceSubresourceRange;

		vkCmdPipelineBarrier(
			frameInfo.commandBuffer,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);
	}
}