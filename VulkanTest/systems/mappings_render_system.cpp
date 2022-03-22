#include "mappings_render_system.hpp"
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

	//Add here descriptor set
	MappingsRenderSystem::MappingsRenderSystem(Vk3dDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout mappingsSetLayout) : vk3dDevice{ device } {
		createMappingsPipelineLayout(mappingsSetLayout);
		createMappingsPipeline(renderPass);
	}

	MappingsRenderSystem::~MappingsRenderSystem() {
		vkDestroyPipelineLayout(vk3dDevice.device(), mappingsPipelineLayout, nullptr);
	}

	void MappingsRenderSystem::createMappingsPipelineLayout(VkDescriptorSetLayout mappingsSetLayout) {
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

	void MappingsRenderSystem::createMappingsPipeline(VkRenderPass renderPass) {
		assert(mappingsPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
		PipelineConfigInfo pipelineConfig{};
		pipelineConfig.attachmentCount = 1;
		pipelineConfig.hasVertexBufferBound = true;
		Vk3dPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.subpass = 0;
		pipelineConfig.pipelineLayout = mappingsPipelineLayout;
		vk3dMappingsPipeline = std::make_unique<Vk3dPipeline>(
			vk3dDevice,
			"shaders/mappings_shader.vert.spv",
			"shaders/mappings_shader.frag.spv",
			pipelineConfig
			);
	}

	void MappingsRenderSystem::renderGameObjects(FrameInfo& frameInfo) {

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

}