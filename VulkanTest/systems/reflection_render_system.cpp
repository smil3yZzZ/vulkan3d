#include "reflection_render_system.hpp"
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

	struct MappingsPushConstantData {
		glm::mat4 modelMatrix{ 1.f };
		glm::mat4 normalMatrix{ 1.f };
	};

	struct UVReflectionMapPushConstantData{
		glm::mat4 modelMatrix{ 1.f };
		float reflection;
	};

	//Add here descriptor set
	ReflectionRenderSystem::ReflectionRenderSystem(Vk3dDevice& device, VkRenderPass mappingsRenderPass, VkDescriptorSetLayout mappingsSetLayout, VkRenderPass uvReflectionMapRenderPass, VkDescriptorSetLayout uvReflectionMapSetLayout) : vk3dDevice{ device } {
		createMappingsPipelineLayout(mappingsSetLayout);
		createMappingsPipeline(mappingsRenderPass);
		createUVReflectionMapPipelineLayout(uvReflectionMapSetLayout);
		createUVReflectionMapPipeline(uvReflectionMapRenderPass);
	}

	ReflectionRenderSystem::~ReflectionRenderSystem() {
		vkDestroyPipelineLayout(vk3dDevice.device(), mappingsPipelineLayout, nullptr);
	}

	void ReflectionRenderSystem::createMappingsPipelineLayout(VkDescriptorSetLayout mappingsSetLayout) {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(MappingsPushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ mappingsSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(vk3dDevice.device(), &pipelineLayoutInfo, nullptr, &mappingsPipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void ReflectionRenderSystem::createMappingsPipeline(VkRenderPass mappingsRenderPass) {
		assert(mappingsPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
		PipelineConfigInfo pipelineConfig{};
		pipelineConfig.attachmentCount = 1;
		pipelineConfig.hasVertexBufferBound = true;
		Vk3dPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = mappingsRenderPass;
		pipelineConfig.subpass = 0;
		pipelineConfig.pipelineLayout = mappingsPipelineLayout;
		vk3dMappingsPipeline = std::make_unique<Vk3dPipeline>(
			vk3dDevice,
			"shaders/mappings_shader.vert.spv",
			"shaders/mappings_shader.frag.spv",
			pipelineConfig
			);
	}

	void ReflectionRenderSystem::createUVReflectionMapPipelineLayout(VkDescriptorSetLayout uvReflectionMapSetLayout) {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(UVReflectionMapPushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ uvReflectionMapSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(vk3dDevice.device(), &pipelineLayoutInfo, nullptr, &uvReflectionMapPipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void ReflectionRenderSystem::createUVReflectionMapPipeline(VkRenderPass uvReflectionMapRenderPass) {
		assert(mappingsPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
		PipelineConfigInfo pipelineConfig{};
		pipelineConfig.attachmentCount = 1;
		pipelineConfig.hasVertexBufferBound = true;
		Vk3dPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = uvReflectionMapRenderPass;
		pipelineConfig.subpass = 0;
		pipelineConfig.pipelineLayout = uvReflectionMapPipelineLayout;
		vk3dUVReflectionMapPipeline = std::make_unique<Vk3dPipeline>(
			vk3dDevice,
			"shaders/uv_reflection_shader.vert.spv",
			"shaders/uv_reflection_shader.frag.spv",
			pipelineConfig
			);
	}

	void ReflectionRenderSystem::renderMappings(FrameInfo& frameInfo) {
		vk3dMappingsPipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			mappingsPipelineLayout,
			0,
			1,
			&frameInfo.mappingsDescriptorSet,
			0,
			nullptr);

		for (auto& kv : frameInfo.gameObjects) {
			auto& obj = kv.second;

			MappingsPushConstantData push{};

			push.modelMatrix = obj.transform.mat4();
			push.normalMatrix = obj.transform.normalMatrix();

			vkCmdPushConstants(
				frameInfo.commandBuffer,
				mappingsPipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT,
				0,
				sizeof(MappingsPushConstantData),
				&push
			);

			obj.model->bind(frameInfo.commandBuffer);
			obj.model->draw(frameInfo.commandBuffer);
		}
	}

	void ReflectionRenderSystem::renderUVReflectionMap(FrameInfo& frameInfo) {
		vk3dUVReflectionMapPipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			uvReflectionMapPipelineLayout,
			0,
			1,
			&frameInfo.uvReflectionDescriptorSet,
			0,
			nullptr);

		for (auto& kv : frameInfo.gameObjects) {
			auto& obj = kv.second;

			UVReflectionMapPushConstantData push{};

			push.modelMatrix = obj.transform.mat4();
			push.reflection = obj.reflection;
			
			vkCmdPushConstants(
				frameInfo.commandBuffer,
				uvReflectionMapPipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(UVReflectionMapPushConstantData),
				&push
			);

			obj.model->bind(frameInfo.commandBuffer);
			obj.model->draw(frameInfo.commandBuffer);
		}
	}

}