#include "simple_render_system.hpp"
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

	struct SimplePushConstantData {
		glm::mat4 modelMatrix{ 1.f };
		glm::mat4 normalMatrix{ 1.f };
	};

	SimpleRenderSystem::SimpleRenderSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout gBufferSetLayout, VkDescriptorSetLayout compositionSetLayout) : lveDevice{device} {
		createGBufferPipelineLayout(gBufferSetLayout);
		createGBufferPipeline(renderPass);
		createCompositionPipelineLayout(compositionSetLayout);
		createCompositionPipeline(renderPass);
	}

	SimpleRenderSystem::~SimpleRenderSystem() {
		vkDestroyPipelineLayout(lveDevice.device(), compositionPipelineLayout, nullptr);
		vkDestroyPipelineLayout(lveDevice.device(), gBufferPipelineLayout, nullptr);
	}

	void SimpleRenderSystem::createGBufferPipelineLayout(VkDescriptorSetLayout gBufferSetLayout) {

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ gBufferSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &gBufferPipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void SimpleRenderSystem::createGBufferPipeline(VkRenderPass renderPass) {
		assert(gBufferPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
		PipelineConfigInfo pipelineConfig{};
		LvePipeline::defaultPipelineConfigInfo(pipelineConfig, true);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.subpass = 0;
		pipelineConfig.pipelineLayout = gBufferPipelineLayout;
		lveGBufferPipeline = std::make_unique<LvePipeline>(
			lveDevice,
			"shaders/gbuffer_shader.vert.spv",
			"shaders/gbuffer_shader.frag.spv",
			pipelineConfig
			);
	}

	void SimpleRenderSystem::createCompositionPipelineLayout(VkDescriptorSetLayout compositionSetLayout) {
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ compositionSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
		if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &compositionPipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void SimpleRenderSystem::createCompositionPipeline(VkRenderPass renderPass) {
		assert(compositionPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
		PipelineConfigInfo pipelineConfig{};
		LvePipeline::defaultPipelineConfigInfo(pipelineConfig, false);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.subpass = 1;
		pipelineConfig.pipelineLayout = compositionPipelineLayout;
		lveCompositionPipeline = std::make_unique<LvePipeline>(
			lveDevice,
			"shaders/composition_shader.vert.spv",
			"shaders/composition_shader.frag.spv",
			pipelineConfig
			);
	}

	void SimpleRenderSystem::renderGameObjects(FrameInfo& frameInfo) {
		// First subpass
		lveGBufferPipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			gBufferPipelineLayout,
			0,
			1,
			&frameInfo.gBufferDescriptorSet,
			0,
			nullptr);

		for (auto& kv : frameInfo.gameObjects) {
			auto& obj = kv.second;
			//obj.transform.rotation.y = glm::mod(obj.transform.rotation.y + 0.01f, glm::two_pi<float>());
			//obj.transform.rotation.x = glm::mod(obj.transform.rotation.y + 0.001f, glm::two_pi<float>());

			SimplePushConstantData push{};

			push.modelMatrix = obj.transform.mat4();
			push.normalMatrix = obj.transform.normalMatrix();

			vkCmdPushConstants(
				frameInfo.commandBuffer,
				gBufferPipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push
			);

			obj.model->bind(frameInfo.commandBuffer);
			obj.model->draw(frameInfo.commandBuffer);
		}

		//Subpass transition
		vkCmdNextSubpass(frameInfo.commandBuffer, VK_SUBPASS_CONTENTS_INLINE);

		//Second subpass		
		lveCompositionPipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			compositionPipelineLayout,
			0,
			1,
			&frameInfo.compositionDescriptorSet,
			0,
			nullptr);

		vkCmdDraw(frameInfo.commandBuffer, 3, 1, 0, 0);
	}

}