#ifndef __switch__
#include <glad/gl.h>
#endif

#include "state.h"

#include "../engine/console.h"
#include "program.h"
#include "shader.h"
#include "vertexAttributes.h"
#include "vertexBuffer.h"
#include "window.h"

tsl::robin_map<render::Program*, uint32_t> render::State::DescriptorSetIndex = tsl::robin_map<render::Program*, uint32_t>();
tsl::robin_map<std::pair<render::Program*, std::string>, uint32_t> render::State::UniformIndex = tsl::robin_map<std::pair<render::Program*, std::string>, uint32_t>();

render::State::State() {

}

render::State::State(render::Window* window) {
	this->window = window;
}

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

	if(this->old.program != this->current.program) {
		this->descriptorSetState = DESCRIPTOR_SET_NEEDS_REALLOC;
		this->topDescriptor = 0;
	}
	
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

void render::State::reset() {
	this->applied = false;

	#ifdef __switch__
	#else
	if(this->window->backend != VULKAN_BACKEND) {
		return;
	}

	this->buffer[this->window->framePingPong].reset();
	this->oldPipeline = {};
	this->descriptorSetState = DESCRIPTOR_SET_NEEDS_REALLOC; // reset in-use flag to true so we re-allocate descriptor set

	State::DescriptorSetIndex.clear();
	State::UniformIndex.clear();
	#endif
}

void render::State::bindPipeline() {
	#ifndef __switch__
	if(this->window->backend != VULKAN_BACKEND || this->window->swapchainOutOfDate) {
		return;
	}

	render::VulkanPipeline pipeline = {
		this->window,
		this->current.primitive,
		(float)this->window->width,
		(float)this->window->height,
		this->current.program,
		this->current.attributes,
	};

	if(this->current != this->old || !this->applied) {
		if(this->current.program == nullptr) {
			console::print("render state: no program bound\n");
			exit(1);
		}

		if(this->window->pipelines.find(pipeline) == this->window->pipelines.end()) {
			this->window->pipelines[pipeline] = pipeline.newPipeline(); // TODO move this creation step to the window class??
		}

		if(this->oldPipeline != pipeline) {
			this->buffer[this->window->framePingPong].bindPipeline(
				vk::PipelineBindPoint::eGraphics, this->window->pipelines[pipeline].pipeline
			);
		}
		this->oldPipeline = pipeline;

		// probably should move vertex buffer binding away from here
		std::vector<vk::Buffer> vertexBuffers;
		std::vector<vk::DeviceSize> offsets;
		for(VertexAttribute attribute: this->current.attributes->attributes) {
			// figure out if we need to copy buffer contents
			if(attribute.buffer->needsCopy) {
				this->window->copyVulkanBuffer(attribute.buffer->stagingBuffer, attribute.buffer->gpuBuffer);
				attribute.buffer->needsCopy = false;
			}

			if(attribute.buffer->isDynamicDraw) {
				if(attribute.buffer->isOutOfDateBuffer()) {
					attribute.buffer->handleOutOfDateBuffer();
					VertexBuffer::OutOfDateBuffers[this->window->framePingPong].erase(attribute.buffer);
				}
				
				vertexBuffers.push_back(attribute.buffer->dynamicBuffers[this->window->framePingPong].buffer->getBuffer());
			}
			else {
				vertexBuffers.push_back(attribute.buffer->gpuBuffer->getBuffer());
			}
			offsets.push_back(0);
		}

		this->buffer[this->window->framePingPong].bindVertexBuffers(0, vertexBuffers.size(), vertexBuffers.data(), offsets.data());
		this->old = this->current;
		this->applied = true;
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
			this->window->pipelines[pipeline].layout,
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

		info.imageInfo = vk::DescriptorImageInfo(
			this->descriptorWrites[this->current.program->uniformToShaderBinding[uniformName]].texture->sampler,
			this->descriptorWrites[this->current.program->uniformToShaderBinding[uniformName]].texture->imageView,
			vk::ImageLayout::eShaderReadOnlyOptimal
		);
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
#endif
