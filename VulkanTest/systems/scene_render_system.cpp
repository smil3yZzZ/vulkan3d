#include "scene_render_system.hpp"
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

	struct GBufferPushConstantData {
		glm::mat4 modelMatrix{ 1.f };
		glm::mat4 normalMatrix{ 1.f };
	};

	struct CompositionPushConstantData {
		glm::mat4 invViewProj{ 1.f };
		glm::vec2 invResolution{ 1.f };
	};

	SceneRenderSystem::SceneRenderSystem(Vk3dDevice& device, VkRenderPass lightingRenderPass, VkDescriptorSetLayout gBufferSetLayout, 
		VkDescriptorSetLayout compositionSetLayout, VkRenderPass postProcessingRenderPass, VkDescriptorSetLayout postProcessingSetLayout) : vk3dDevice{device} {
		createGBufferPipelineLayout(gBufferSetLayout);
		createGBufferPipeline(lightingRenderPass);
		createCompositionPipelineLayout(compositionSetLayout);
		createCompositionPipeline(lightingRenderPass);
		createPostProcessingPipelineLayout(postProcessingSetLayout);
		createPostProcessingPipeline(postProcessingRenderPass);
	}

	SceneRenderSystem::~SceneRenderSystem() {
		vkDestroyPipelineLayout(vk3dDevice.device(), postProcessingPipelineLayout, nullptr);
		vkDestroyPipelineLayout(vk3dDevice.device(), compositionPipelineLayout, nullptr);
		vkDestroyPipelineLayout(vk3dDevice.device(), gBufferPipelineLayout, nullptr);
	}

	void SceneRenderSystem::createGBufferPipelineLayout(VkDescriptorSetLayout gBufferSetLayout) {

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(GBufferPushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ gBufferSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(vk3dDevice.device(), &pipelineLayoutInfo, nullptr, &gBufferPipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void SceneRenderSystem::createGBufferPipeline(VkRenderPass lightingRenderPass) {
		assert(gBufferPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
		PipelineConfigInfo pipelineConfig{};
		pipelineConfig.attachmentCount = 2;
		pipelineConfig.hasVertexBufferBound = true;
		Vk3dPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = lightingRenderPass;
		pipelineConfig.subpass = 0;
		pipelineConfig.pipelineLayout = gBufferPipelineLayout;
		vk3dGBufferPipeline = std::make_unique<Vk3dPipeline>(
			vk3dDevice,
			"shaders/gbuffer_shader.vert.spv",
			"shaders/gbuffer_shader.frag.spv",
			pipelineConfig
			);
	}

	void SceneRenderSystem::createCompositionPipelineLayout(VkDescriptorSetLayout compositionSetLayout) {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(CompositionPushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ compositionSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(vk3dDevice.device(), &pipelineLayoutInfo, nullptr, &compositionPipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void SceneRenderSystem::createCompositionPipeline(VkRenderPass lightingRenderPass) {
		assert(compositionPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
		PipelineConfigInfo pipelineConfig{};
		pipelineConfig.attachmentCount = 1;
		pipelineConfig.hasVertexBufferBound = false;
		Vk3dPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = lightingRenderPass;
		pipelineConfig.subpass = 1;
		pipelineConfig.pipelineLayout = compositionPipelineLayout;
		vk3dCompositionPipeline = std::make_unique<Vk3dPipeline>(
			vk3dDevice,
			"shaders/composition_shader.vert.spv",
			"shaders/composition_shader.frag.spv",
			pipelineConfig
			);
	}

	void SceneRenderSystem::createPostProcessingPipelineLayout(VkDescriptorSetLayout postProcessingSetLayout) {
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ postProcessingSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		if (vkCreatePipelineLayout(vk3dDevice.device(), &pipelineLayoutInfo, nullptr, &postProcessingPipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void SceneRenderSystem::createPostProcessingPipeline(VkRenderPass postProcessingRenderPass) {
		assert(postProcessingPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
		PipelineConfigInfo pipelineConfig{};
		pipelineConfig.attachmentCount = 1;
		pipelineConfig.hasVertexBufferBound = false;
		Vk3dPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = postProcessingRenderPass;
		pipelineConfig.subpass = 0;
		pipelineConfig.pipelineLayout = postProcessingPipelineLayout;
		vk3dPostProcessingPipeline = std::make_unique<Vk3dPipeline>(
			vk3dDevice,
			"shaders/post_processing_shader.vert.spv",
			"shaders/post_processing_shader.frag.spv",
			pipelineConfig
			);
	}

	void SceneRenderSystem::renderGameObjectsLighting(FrameInfo& frameInfo, glm::mat4 invViewProj, glm::vec2 invResolution) {
		// First subpass
		vk3dGBufferPipeline->bind(frameInfo.commandBuffer);

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

			GBufferPushConstantData push{};

			push.modelMatrix = obj.transform.mat4();
			push.normalMatrix = obj.transform.normalMatrix();

			vkCmdPushConstants(
				frameInfo.commandBuffer,
				gBufferPipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT,
				0,
				sizeof(GBufferPushConstantData),
				&push
			);

			obj.model->bind(frameInfo.commandBuffer);
			obj.model->draw(frameInfo.commandBuffer);
		}

		//Subpass transition
		vkCmdNextSubpass(frameInfo.commandBuffer, VK_SUBPASS_CONTENTS_INLINE);

		//Second subpass		
		vk3dCompositionPipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			compositionPipelineLayout,
			0,
			1,
			&frameInfo.compositionDescriptorSet,
			0,
			nullptr);

		CompositionPushConstantData push{};

		push.invViewProj = invViewProj;
		push.invResolution = invResolution;

		vkCmdPushConstants(
			frameInfo.commandBuffer,
			compositionPipelineLayout,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(CompositionPushConstantData),
			&push
		);

		vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
	}

	void SceneRenderSystem::renderPostProcessing(FrameInfo& frameInfo) {
		vk3dPostProcessingPipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			postProcessingPipelineLayout,
			0,
			1,
			&frameInfo.postProcessingDescriptorSet,
			0,
			nullptr);

		vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
	}

}