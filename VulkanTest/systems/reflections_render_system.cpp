#include "reflections_render_system.hpp"
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

	struct ReflectionsPushConstantData {
		glm::mat4 modelMatrix{ 1.f };
		glm::mat4 normalMatrix{ 1.f };
	};

	//Add here descriptor set
	ReflectionsRenderSystem::ReflectionsRenderSystem(Vk3dDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout reflectionsSetLayout) : vk3dDevice{ device } {
		createReflectionsPipelineLayout(reflectionsSetLayout);
		createReflectionsPipeline(renderPass);
	}

	ReflectionsRenderSystem::~ReflectionsRenderSystem() {
		vkDestroyPipelineLayout(vk3dDevice.device(), reflectionsPipelineLayout, nullptr);
	}

	void ReflectionsRenderSystem::createReflectionsPipelineLayout(VkDescriptorSetLayout reflectionsSetLayout) {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(ReflectionsPushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ reflectionsSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		if (vkCreatePipelineLayout(vk3dDevice.device(), &pipelineLayoutInfo, nullptr, &reflectionsPipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void ReflectionsRenderSystem::createReflectionsPipeline(VkRenderPass renderPass) {
		assert(reflectionsPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
		PipelineConfigInfo pipelineConfig{};
		pipelineConfig.attachmentCount = 1;
		pipelineConfig.hasVertexBufferBound = true;
		Vk3dPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.subpass = 0;
		pipelineConfig.pipelineLayout = reflectionsPipelineLayout;
		vk3dReflectionsPipeline = std::make_unique<Vk3dPipeline>(
			vk3dDevice,
			"shaders/reflections_shader.vert.spv",
			"shaders/reflections_shader.frag.spv",
			pipelineConfig
			);
	}

	void ReflectionsRenderSystem::renderGameObjects(FrameInfo& frameInfo) {

		vk3dReflectionsPipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			reflectionsPipelineLayout,
			0,
			1,
			&frameInfo.reflectionsDescriptorSet,
			0,
			nullptr);

		for (auto& kv : frameInfo.gameObjects) {
			auto& obj = kv.second;

			ReflectionsPushConstantData push{};

			push.modelMatrix = obj.transform.mat4();
			push.normalMatrix = obj.transform.normalMatrix();

			vkCmdPushConstants(
				frameInfo.commandBuffer,
				reflectionsPipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT,
				0,
				sizeof(ReflectionsPushConstantData),
				&push
			);

			obj.model->bind(frameInfo.commandBuffer);
			obj.model->draw(frameInfo.commandBuffer);
		}
	}

}