#ifndef __switch__
#include "vulkanPipeline.h"

#include "program.h"
#include "vertexAttributes.h"
#include "window.h"

#include "debug.h"

render::VulkanPipelineResult render::VulkanPipeline::newPipeline() {
	VulkanPipelineResult output;

	// handle pipeline layout
	{
		std::vector<vk::DescriptorSetLayout> descriptorLayouts;
		descriptorLayouts.push_back(this->program->descriptorLayout);
		
		vk::PipelineLayoutCreateInfo pipelineLayoutInfo(
			{}, descriptorLayouts.size(), descriptorLayouts.data(), 0, nullptr
		);

		vk::Result result = this->window->device.device.createPipelineLayout(&pipelineLayoutInfo, nullptr, &output.layout); // TODO remember to clean up
		if(result != vk::Result::eSuccess) {
			console::error("vulkan: could not create pipeline layout: %s\n", vkResultToString((VkResult)result).c_str());
			exit(1);
		}
	}
	
	// handle pipeline
	{
		vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
		vk::PipelineVertexInputDivisorStateCreateInfoEXT divisorInfo;
		if(this->attributes != nullptr) {
			vertexInputInfo = vk::PipelineVertexInputStateCreateInfo(
				{},
				(uint32_t)this->attributes->inputBindings.size(),
				this->attributes->inputBindings.data(),
				(uint32_t)this->attributes->inputAttributes.size(),
				this->attributes->inputAttributes.data()
			);

			if(this->attributes->inputDivisors.size() > 0) {
				divisorInfo = vk::PipelineVertexInputDivisorStateCreateInfoEXT(
					(uint32_t)this->attributes->inputDivisors.size(),
					this->attributes->inputDivisors.data()
				);
				vertexInputInfo.setPNext(&divisorInfo);
			}
		}

		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo({}, primitiveToVulkanPrimitive(this->topology), false);
		vk::Viewport viewport(0.0f, this->viewportHeight, this->viewportWidth, -this->viewportHeight, 0.0f, 1.0f);
		vk::Rect2D scissor({ 0, 0 }, this->window->swapchainExtent);

		vk::PipelineViewportStateCreateInfo viewportStateInfo({}, 1, &viewport, 1, &scissor);
		vk::PipelineRasterizationStateCreateInfo rasterizationInfo(
			{},
			false,
			false,
			vk::PolygonMode::eFill, // TODO change fill based on primitive?
			vk::CullModeFlagBits::eNone,
			vk::FrontFace::eCounterClockwise,
			false,
			0.0f,
			0.0f,
			0.0f,
			1.0f
		);

		vk::PipelineMultisampleStateCreateInfo multisampleInfo({}, vk::SampleCountFlagBits::e1, false, 1.0f, nullptr, false, false);

		vk::PipelineColorBlendAttachmentState colorBlend(
			true,
			vk::BlendFactor::eSrcAlpha, // color blend
			vk::BlendFactor::eOneMinusSrcAlpha,
			vk::BlendOp::eAdd,
			vk::BlendFactor::eOne, // alpha blend
			vk::BlendFactor::eZero,
			vk::BlendOp::eAdd,
			vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA 
		);

		vk::PipelineColorBlendStateCreateInfo colorBlendInfo(
			{},
			false,
			vk::LogicOp::eCopy,
			1,
			&colorBlend,
			{ 0.0f, 0.0f, 0.0f, 0.0f, }
		);

		vk::PipelineDynamicStateCreateInfo dynamicStateInfo({}, 0, nullptr);

		vk::StencilOpState stencilOpState = vk::StencilOpState(
			stencilOPToVulkanStencilOP(this->stencilFail),
			stencilOPToVulkanStencilOP(this->stencilPass),
			stencilOPToVulkanStencilOP(this->depthFail),
			stencilToVulkanStencil(this->stencilFunction),
			this->stencilCompare,
			this->stencilWriteMask,
			this->stencilReference
		);

		vk::PipelineDepthStencilStateCreateInfo depthInfo(
			{},
			this->depthEnabled, // depth test enabled
			true, // depth write enabled
			vk::CompareOp::eLessOrEqual,
			false,
			this->stencilEnabled,
			stencilOpState,
			stencilOpState,
			0.0f,
			1.0f
		);

		// create the pipeline
		vk::GraphicsPipelineCreateInfo pipelineInfo(
			{},
			this->program->stages.size(),
			this->program->stages.data(),
			&vertexInputInfo,
			&inputAssemblyInfo,
			nullptr,
			&viewportStateInfo,
			&rasterizationInfo,
			&multisampleInfo,
			&depthInfo,
			&colorBlendInfo,
			&dynamicStateInfo,
			output.layout,
			this->window->renderPass,
			0
		);

		vk::Result result = this->window->device.device.createGraphicsPipelines(this->window->pipelineCache, 1, &pipelineInfo, nullptr, &output.pipeline);
		if(result != vk::Result::eSuccess) {
			console::error("vulkan: could not create pipeline: %s\n", vkResultToString((VkResult)result).c_str());
			exit(1);
		}
	}

	return output;
}
#endif
