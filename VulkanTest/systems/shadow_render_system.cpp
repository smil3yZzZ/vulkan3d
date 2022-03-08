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


namespace vk3d {

	struct ShadowPushConstantData {
		glm::mat4 modelMatrix{ 1.f };
	};

	//Add here descriptor set
	ShadowRenderSystem::ShadowRenderSystem(Vk3dDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout shadowSetLayout) : lveDevice{ device } {
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
		Vk3dPipeline::shadowPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.subpass = 0;
		pipelineConfig.pipelineLayout = shadowPipelineLayout;
		lveShadowPipeline = std::make_unique<Vk3dPipeline>(
			lveDevice,
			"shaders/shadow_shader.vert.spv",
			"shaders/shadow_shader.frag.spv",
			pipelineConfig
			);
	}

	void ShadowRenderSystem::renderGameObjects(FrameInfo& frameInfo) {

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

}