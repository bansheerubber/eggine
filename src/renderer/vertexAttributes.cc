#ifndef __switch__
#include <glad/gl.h>
#endif

#include "vertexAttributes.h"
#include "vertexBuffer.h"

#include "window.h"

render::VertexAttributes::VertexAttributes(Window* window) {
	this->window = window;
}

void render::VertexAttributes::addVertexAttribute(class VertexBuffer* buffer, unsigned short attributeLocation, unsigned short vectorLength, VertexAttributeType type, unsigned short offset, unsigned short stride, unsigned short divisor) {
	this->attributes.push_back(VertexAttribute {
		buffer: buffer,
		attributeLocation: attributeLocation,
		vectorLength: vectorLength,
		type: type,
		offset: offset,
		stride: stride,
		divisor: divisor,
	});
}

#ifndef __switch__
std::vector<vk::Buffer> render::VertexAttributes::getVulkanBuffers() {
	std::vector<vk::Buffer> vertexBuffers;
	for(VertexAttribute attribute: this->attributes) {
		// figure out if we need to copy buffer contents
		if(attribute.buffer->needsCopy) {
			this->window->copyVulkanBuffer(attribute.buffer->stagingBuffer, attribute.buffer->gpuBuffer);
			attribute.buffer->needsCopy = false;
		}

		if(attribute.buffer->isDynamicDraw) {
			if(attribute.buffer->isOutOfDateBuffer()) {
				attribute.buffer->handleOutOfDateBuffer();
				VertexBuffer::RemoveOutOfDateBuffer(attribute.buffer);
			}
			
			vertexBuffers.push_back(attribute.buffer->getVulkanDynamicBuffer());
		}
		else {
			vertexBuffers.push_back(attribute.buffer->getVulkanBuffer());
		}
	}
	return vertexBuffers;
}

vk::PipelineVertexInputStateCreateInfo render::VertexAttributes::getVulkanVertexInputInfo() {
	this->vertexInputInfo = vk::PipelineVertexInputStateCreateInfo(
		{},
		(uint32_t)this->inputBindings.size(),
		this->inputBindings.data(),
		(uint32_t)this->inputAttributes.size(),
		this->inputAttributes.data()
	);

	if(this->inputDivisors.size() > 0) {
		this->divisorInfo = vk::PipelineVertexInputDivisorStateCreateInfoEXT(
			(uint32_t)this->inputDivisors.size(),
			this->inputDivisors.data()
		);
		this->vertexInputInfo.setPNext(&this->divisorInfo);
	}

	return this->vertexInputInfo;
}
#endif

void render::VertexAttributes::buildCommandLists() {
	#ifdef __switch__
	this->bufferBindOrder.clear();
	this->attributeStates.clear();
	this->bufferStates.clear();
	
	unsigned short bufferId = 0;
	unsigned short nextBufferId = 0;
	VertexBuffer* currentBuffer = nullptr;
	
	// iterate through the attributes and build the command lists
	for(VertexAttribute &attribute: this->attributes) {
		if(currentBuffer != attribute.buffer) {
			currentBuffer = attribute.buffer;
			this->bufferBindOrder.push_back(currentBuffer);
			this->bufferStates.push_back(DkVtxBufferState {
				attribute.stride,
				attribute.divisor,
			});
			bufferId = nextBufferId;
			nextBufferId++;
		}

		this->attributeStates.push_back(DkVtxAttribState {
			bufferId,
			0,
			attribute.offset,
			attributeTypeToDkAttribSize(attribute.type, attribute.vectorLength),
			attributeTypeToDkAttribType(attribute.type),
			0,
		});
	}
	#else
	if(this->window->backend == OPENGL_BACKEND) {
		if(this->vertexArrayObject != GL_INVALID_INDEX) {
			return;
		}

		glGenVertexArrays(1, &this->vertexArrayObject);
		glBindVertexArray(this->vertexArrayObject);
		
		for(VertexAttribute &attribute: this->attributes) {
			glBindBuffer(GL_ARRAY_BUFFER, attribute.buffer->bufferId);
			if(attribute.type == VERTEX_ATTRIB_HALF_FLOAT || attribute.type == VERTEX_ATTRIB_FLOAT || attribute.type == VERTEX_ATTRIB_DOUBLE) {
				glVertexAttribPointer(attribute.attributeLocation, attribute.vectorLength, attributeTypeToGLType(attribute.type), GL_FALSE, attribute.stride, 0);
			}
			else {
				glVertexAttribIPointer(attribute.attributeLocation, attribute.vectorLength, attributeTypeToGLType(attribute.type), attribute.stride, 0);
			}

			if(attribute.divisor) {
				glVertexAttribDivisor(attribute.attributeLocation, attribute.divisor);
			}

			glEnableVertexAttribArray(attribute.attributeLocation);
		}

		glBindVertexArray(0);
	}
	else {
		if(this->inputBindings.size() == 0) {
			unsigned short bufferId = 0;
			VertexBuffer* currentBuffer = nullptr;
			bool incrementBufferId = this->attributes[0].buffer;

			for(VertexAttribute &attribute: this->attributes) {
				if(currentBuffer != attribute.buffer) {
					this->inputBindings.push_back(vk::VertexInputBindingDescription(
						bufferId,
						attribute.stride,
						attribute.divisor == 0 ? vk::VertexInputRate::eVertex : vk::VertexInputRate::eInstance
					));
					incrementBufferId = true;

					if(attribute.divisor != 0) {
						this->inputDivisors.push_back(vk::VertexInputBindingDivisorDescriptionEXT(
							bufferId,
							attribute.divisor
						));
					}

					currentBuffer = attribute.buffer;
				}

				this->inputAttributes.push_back(vk::VertexInputAttributeDescription(
					attribute.attributeLocation,
					bufferId,
					attributeTypeToVulkanType(attribute.type, attribute.vectorLength),
					attribute.offset
				));

				if(incrementBufferId) {
					bufferId++;
				}
			}
		}
	}
	#endif
}
