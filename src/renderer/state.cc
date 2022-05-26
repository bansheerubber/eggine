#ifndef __switch__
#include <glad/gl.h>
#endif

#include "state.h"

#include "../engine/console.h"
#include "../engine/engine.h"
#include "program.h"
#include "shader.h"
#include "vertexAttributes.h"
#include "vertexBuffer.h"
#include "window.h"

#include "debug.h"

#ifndef __switch__
tsl::robin_map<render::Program*, uint32_t> render::State::DescriptorSetIndex = tsl::robin_map<render::Program*, uint32_t>();
tsl::robin_map<std::pair<render::Program*, std::string>, uint32_t> render::State::UniformIndex = tsl::robin_map<std::pair<render::Program*, std::string>, uint32_t>();
tsl::robin_map<render::SubState, render::VulkanPipelineResult> render::State::VulkanPipelines = tsl::robin_map<render::SubState, render::VulkanPipelineResult>();
#endif

render::State::State() {

}

render::State::State(render::Window* window) {
	this->window = window;
}

#ifdef EGGINE_DEVELOPER_MODE
void render::State::drawImgui() {
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), this->buffer[this->window->framePingPong]);
	this->bindPipeline(true);
}
#endif

void render::State::draw(
	PrimitiveType type, unsigned int firstVertex, unsigned int vertexCount, unsigned int firstInstance, unsigned int instanceCount
) {
	this->current.primitive = type;

	#ifdef __switch__
	this->window->commandBuffer.draw(primitiveToDkPrimitive(type), vertexCount, instanceCount, firstVertex, firstInstance);
	#else
	if(this->window->backend == OPENGL_BACKEND) {
		glDrawArraysInstancedBaseInstance(primitiveToGLPrimitive(type), firstVertex, vertexCount, instanceCount, firstInstance);
	}
	else if(!this->window->swapchainOutOfDate) {
		this->bindPipeline();
		this->buffer[this->window->framePingPong].draw(vertexCount, instanceCount, firstVertex, firstInstance);
	}
	#endif
}

void render::State::bindProgram(render::Program* program) {
	this->current.program = program;
	
	#ifdef __switch__
	std::vector<DkShader const*> shaders;
	for(Shader* shader: program->shaders) {
		shaders.push_back(&shader->shader);
	}
	
	dkCmdBufBindShaders(this->window->commandBuffer, DkStageFlag_GraphicsMask, shaders.data(), shaders.size());

	for(Shader* shader: program->shaders) {
		for(auto &[uniform, binding]: shader->uniformToBinding) {
			program->uniformToBinding[uniform] = binding;
		}
	}
	#else
	if(this->old.program != this->current.program) {
		this->descriptorSetState = DESCRIPTOR_SET_NEEDS_REALLOC;
		this->topDescriptor = 0;
	}

	program->compile();
	if(this->window->backend == OPENGL_BACKEND) {
		glUseProgram(this->current.program->program);
	}
	#endif
}

void render::State::bindUniform(std::string uniformName, void* data, uint32_t size) {
	#ifdef __switch__
	if(this->current.program->uniformToPiece.find(uniformName) == this->current.program->uniformToPiece.end()) {
		this->current.program->createUniformBuffer(uniformName, size);
	}

	this->window->commandBuffer.pushConstants(
		this->current.program->uniformToPiece[uniformName]->gpuAddr(),
		this->current.program->uniformToPiece[uniformName]->size(),
		0,
		size,
		data
	);

	// look for binding
	for(Shader* shader: this->current.program->shaders) {
		if(shader->uniformToBinding.find(uniformName) != shader->uniformToBinding.end()) {
			this->window->commandBuffer.bindUniformBuffer(
				shader->type == SHADER_FRAGMENT ? DkStage_Fragment : DkStage_Vertex,
				shader->uniformToBinding[uniformName],
				this->current.program->uniformToPiece[uniformName]->gpuAddr(),
				this->current.program->uniformToPiece[uniformName]->size()
			);
		}
	}
	#else
	// TODO replace this junk with a bindUniform function on the program
	if(this->window->backend == OPENGL_BACKEND) {
		auto found = this->current.program->uniformToBuffer.find(uniformName);
		if(found == this->current.program->uniformToBuffer.end()) {
			this->current.program->createUniformBuffer(uniformName, size);
			found = this->current.program->uniformToBuffer.find(uniformName);
		}

		glBindBuffer(GL_UNIFORM_BUFFER, found.value());
		// glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_DYNAMIC_DRAW); // orphan the buffer
		glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
	}
	else if(this->window->backend == VULKAN_BACKEND) {
		unsigned int index = State::UniformIndex[std::pair(this->current.program, uniformName)];
		this->current.program->createUniformBuffer(uniformName, size, index);

		Piece* uniform = this->current.program->uniformToVulkanBuffer[std::pair(uniformName, index)].pieces[this->window->framePingPong];
		memcpy(uniform->map(), data, size);	
		this->descriptorWrites[this->current.program->uniformToShaderBinding[uniformName]].uniform = uniform;
		this->updateDescriptorWrites(uniformName);
	
		State::UniformIndex[std::pair(this->current.program, uniformName)]++;
	}
	#endif
}

void render::State::bindTexture(std::string uniformName, render::Texture* texture) {
	#ifdef __switch__
	this->window->commandBuffer.bindTextures(DkStage_Fragment, 0, dkMakeTextureHandle(0, 0));
	#else
	if(this->window->backend == OPENGL_BACKEND) {
		texture->bind(0);
		
		// get the location of the uniform we're going to bind to
		GLuint location = glGetUniformLocation(this->current.program->program, uniformName.c_str());
		glUniform1i(location, 0);
	}
	else if(this->window->backend == VULKAN_BACKEND) {
		this->descriptorWrites[this->current.program->uniformToShaderBinding[uniformName]].texture = texture;
		this->updateDescriptorWrites(uniformName);
	}
	#endif
}

void render::State::bindVertexAttributes(render::VertexAttributes* attributes) {
	this->current.attributes = attributes;

	attributes->buildCommandLists();

	#ifdef __switch__
	unsigned short id = 0;
	for(VertexBuffer* buffer: attributes->bufferBindOrder) {
		buffer->bind(id++);
	}

	this->window->commandBuffer.bindVtxAttribState(
		dk::detail::ArrayProxy(attributes->attributeStates.size(), (const DkVtxAttribState*)attributes->attributeStates.data())
	);
	this->window->commandBuffer.bindVtxBufferState(
		dk::detail::ArrayProxy(attributes->bufferStates.size(), (const DkVtxBufferState*)attributes->bufferStates.data())
	);
	#else
	if(this->window->backend == OPENGL_BACKEND) {
		glBindVertexArray(attributes->vertexArrayObject);
	}
	#endif
}

void render::State::setStencilFunction(StencilFunction func, unsigned int reference, unsigned int compare) {
	#ifdef __switch__
	#else
	if(this->window->backend == OPENGL_BACKEND) {
		glStencilFunc(stencilToGLStencil(func), reference, compare);
	}
	else {
		this->current.stencilFunction = func;
		this->current.stencilReference = reference;
		this->current.stencilCompare = compare;
	}
	#endif
}

void render::State::setStencilMask(unsigned int mask) {
	#ifdef __switch__
	#else
	if(this->window->backend == OPENGL_BACKEND) {
		glStencilMask(mask);
	}
	else {
		this->current.stencilWriteMask = mask;
	}
	#endif
}

void render::State::setStencilOperation(StencilOperation stencilFail, StencilOperation depthFail, StencilOperation pass) {
	#ifdef __switch__
	#else
	if(this->window->backend == OPENGL_BACKEND) {
		glStencilOp(stencilOPToGLStencilOP(stencilFail), stencilOPToGLStencilOP(depthFail), stencilOPToGLStencilOP(pass));
	}
	else {
		this->current.stencilFail = stencilFail;
		this->current.depthFail = depthFail;
		this->current.stencilPass = pass;
	}
	#endif
}

void render::State::enableStencilTest(bool enable) {
	#ifdef __switch__
	#else
	if(this->window->backend == OPENGL_BACKEND) {
		if(enable) {
			glEnable(GL_STENCIL_TEST);
		}
		else {
			glDisable(GL_STENCIL_TEST);
		}
	}
	else {
		this->current.stencilEnabled = enable;
	}
	#endif
}

void render::State::enableDepthTest(bool enable) {
	#ifdef __switch__
	#else
	if(this->window->backend == OPENGL_BACKEND) {
		if(enable) {
			glEnable(GL_DEPTH_TEST);
		}
		else {
			glDisable(GL_DEPTH_TEST);
		}
	}
	else {
		this->current.depthEnabled = enable;
	}
	#endif
}

void render::State::reset() {
	#ifdef __switch__
	#else
	if(this->window->backend != VULKAN_BACKEND) {
		return;
	}

	this->buffer[this->window->framePingPong].reset(); // reset command buffer
	this->descriptorSetState = DESCRIPTOR_SET_NEEDS_REALLOC; // reset in-use flag to true so we re-allocate descriptor set

	State::DescriptorSetIndex.clear();
	State::UniformIndex.clear();
	#endif
}

void render::State::resize(unsigned int width, unsigned int height) {
	this->current.viewportWidth = width;
	this->current.viewportHeight = height;
}

void render::State::bindPipeline(bool force) {
	#ifndef __switch__
	if(this->window->backend != VULKAN_BACKEND || this->window->swapchainOutOfDate) {
		return;
	}

	if(this->current != this->old || force) {
		if(this->current.program == nullptr) {
			console::print("render state: no program bound\n");
			exit(1);
		}

		if(this->current.attributes == nullptr) {
			console::print("render state: no vertex attributes bound\n");
			exit(1);
		}

		// create the pipeline if we don't have it
		if(State::VulkanPipelines.find(this->current) == State::VulkanPipelines.end()) {
			VulkanPipelineResult output;

			// handle pipeline layout
			{
				std::vector<vk::DescriptorSetLayout> descriptorLayouts;
				descriptorLayouts.push_back(this->current.program->descriptorLayout);
				
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
				vk::PipelineVertexInputStateCreateInfo vertexInputInfo = this->current.attributes->getVulkanVertexInputInfo();
				vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo({}, primitiveToVulkanPrimitive(this->current.primitive), false);
				vk::Viewport viewport(0.0f, this->current.viewportHeight, this->current.viewportWidth, -this->current.viewportHeight, 0.0f, 1.0f);
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
					stencilOPToVulkanStencilOP(this->current.stencilFail),
					stencilOPToVulkanStencilOP(this->current.stencilPass),
					stencilOPToVulkanStencilOP(this->current.depthFail),
					stencilToVulkanStencil(this->current.stencilFunction),
					this->current.stencilCompare,
					this->current.stencilWriteMask,
					this->current.stencilReference
				);

				vk::PipelineDepthStencilStateCreateInfo depthInfo(
					{},
					this->current.depthEnabled, // depth test enabled
					true, // depth write enabled
					vk::CompareOp::eLessOrEqual,
					false,
					this->current.stencilEnabled,
					stencilOpState,
					stencilOpState,
					0.0f,
					1.0f
				);

				// create the pipeline
				vk::GraphicsPipelineCreateInfo pipelineInfo(
					{},
					this->current.program->stages.size(),
					this->current.program->stages.data(),
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

			State::VulkanPipelines[this->current] = output;
		}

		// bind the pipeline
		this->buffer[this->window->framePingPong].bindPipeline(
			vk::PipelineBindPoint::eGraphics, State::VulkanPipelines[this->current].pipeline
		);

		// get the buffer from the vertex attributes
		std::vector<vk::Buffer> vertexBuffers = this->current.attributes->getVulkanBuffers();
		std::vector<vk::DeviceSize> offsets;
		for(uint32_t i = 0; i < vertexBuffers.size(); i++) {
			offsets.push_back(0);
		}

		this->buffer[this->window->framePingPong].bindVertexBuffers(0, vertexBuffers.size(), vertexBuffers.data(), offsets.data());
		this->old = this->current;
	}

	if(this->descriptorSetState == DESCRIPTOR_SET_NEEDS_UPDATE) {
		std::vector<vk::WriteDescriptorSet> writes;
		for(uint32_t i = 0; i < this->topDescriptor + 1; i++) {
			this->descriptorWrites[i].write.setDstSet(
				this->current.program->getDescriptorSet(this->descriptorSetIndex, this->window->framePingPong)
			);
			writes.push_back(this->descriptorWrites[i].write);
		}

		this->window->device.device.updateDescriptorSets(this->topDescriptor + 1, writes.data(), 0, nullptr);

		this->buffer[this->window->framePingPong].bindDescriptorSets(
			vk::PipelineBindPoint::eGraphics,
			State::VulkanPipelines[this->current].layout,
			0,
			1,
			&this->current.program->getDescriptorSet(this->descriptorSetIndex, this->window->framePingPong),
			0,
			nullptr
		);
	}
	this->descriptorSetState = DESCRIPTOR_SET_NEEDS_REALLOC;

	#endif
}

#ifndef __switch__
void render::State::updateDescriptorWrites(std::string uniformName) {
	// figure out descriptor sets
	uint32_t binding = this->current.program->uniformToShaderBinding[uniformName];
	UniformInfo &info = this->descriptorWrites[binding];
	if(this->current.program->isUniformSampler[uniformName]) {
		if(this->descriptorWrites[this->current.program->uniformToShaderBinding[uniformName]].texture == nullptr) {
			console::error("state: could not find texture '%s'\n", uniformName.c_str());
			exit(1);
		}

		info.imageInfo = this->descriptorWrites[this->current.program->uniformToShaderBinding[uniformName]].texture->getVulkanImageInfo();
	}
	else {
		if(this->descriptorWrites[this->current.program->uniformToShaderBinding[uniformName]].uniform == nullptr) {
			console::error("state: could not find uniform '%s'\n", uniformName.c_str());
			exit(1);
		}
		
		info.bufferInfo = vk::DescriptorBufferInfo(
			this->descriptorWrites[this->current.program->uniformToShaderBinding[uniformName]].uniform->getBuffer(),
			0,
			VK_WHOLE_SIZE
		);
	}

	// create a descriptor set if needed
	if(this->descriptorSetState == DESCRIPTOR_SET_NEEDS_REALLOC) {
		this->descriptorSetIndex = State::DescriptorSetIndex[this->current.program];
		this->current.program->createDescriptorSet(this->descriptorSetIndex);
		this->descriptorSetState = DESCRIPTOR_SET_NEEDS_UPDATE;

		State::DescriptorSetIndex[this->current.program]++;
	}

	if(binding > this->topDescriptor) {
		this->topDescriptor = binding;
	}

	info.write = vk::WriteDescriptorSet(
		this->current.program->getDescriptorSet(this->descriptorSetIndex, this->window->framePingPong),
		binding,
		0,
		1,
		this->current.program->isUniformSampler[uniformName] ? vk::DescriptorType::eCombinedImageSampler : vk::DescriptorType::eUniformBuffer,
		this->current.program->isUniformSampler[uniformName] ? &info.imageInfo : nullptr,
		this->current.program->isUniformSampler[uniformName] ? nullptr : &info.bufferInfo,
		nullptr
	);
}

void render::State::ResetPipelines() {
	for(auto &[_, pipeline]: State::VulkanPipelines) {
		engine->renderWindow.device.device.destroyPipelineLayout(pipeline.layout);
		engine->renderWindow.device.device.destroyPipeline(pipeline.pipeline);
	}

	State::VulkanPipelines.clear();
}
#endif
