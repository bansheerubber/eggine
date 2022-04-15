#include "pipeline.h"

#include "window.h"
#include "debug.h"

vk::Pipeline* render::VulkanPipeline::newPipeline() {
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo({}, 0, nullptr, 0, nullptr);
	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo({}, primitiveToVulkanPrimitive(this->topology), false);
	vk::Viewport viewport(0.0f, 0.0f, this->viewportWidth, this->viewportHeight, 0.0f, 1.0f);
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
		false,
		vk::BlendFactor::eOne, // color blend
		vk::BlendFactor::eZero,
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

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, 0, nullptr, 0, nullptr);
	vk::PipelineLayout pipelineLayout = this->window->device.device.createPipelineLayout(pipelineLayoutInfo); // TODO remember to clean up

	// handle renderpass/subpasses
	vk::AttachmentDescription colorAttachment({}, this->window->swapchainFormat.format, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);

	vk::AttachmentReference attachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal); // attach to the output color location in the shader
	vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics, 0, nullptr, 1, &attachmentReference);

	vk::RenderPassCreateInfo renderPassInfo({}, 1, &colorAttachment, 1, &subpass);
	vk::RenderPass renderPass = this->window->device.device.createRenderPass(renderPassInfo); // TODO remember to clean up

	// create the pipeline
	vk::GraphicsPipelineCreateInfo pipelineInfo({}, 0, nullptr, &vertexInputInfo, &inputAssemblyInfo, nullptr, &viewportStateInfo, &rasterizationInfo, &multisampleInfo, nullptr, &colorBlendInfo, &dynamicStateInfo, pipelineLayout, renderPass, 0);

	vk::Pipeline* pipeline = new vk::Pipeline;
	vk::PipelineCache pipelineCache = vk::PipelineCache();
	vk::Result result = this->window->device.device.createGraphicsPipelines(pipelineCache, 1, &pipelineInfo, nullptr, pipeline);

	if(result != vk::Result::eSuccess) {
		console::error("vulkan: could not create pipeline: %s\n", vkResultToString((VkResult)result).c_str());
		exit(1);
	}
	return pipeline;
}
