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

	ShadowRenderSystem::ShadowRenderSystem(LveDevice& device, VkRenderPass renderPass) : lveDevice{ device } {
		createShadowPipelineLayout();
		createShadowPipeline(renderPass);
	}

	ShadowRenderSystem::~ShadowRenderSystem() {
		vkDestroyPipelineLayout(lveDevice.device(), shadowPipelineLayout, nullptr);
	}

	void ShadowRenderSystem::createShadowPipelineLayout() {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(ShadowPushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &shadowPipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void ShadowRenderSystem::createShadowPipeline(VkRenderPass renderPass) {
		assert(shadowPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
		PipelineConfigInfo pipelineConfig{};
		pipelineConfig.attachmentCount = 0;
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

}