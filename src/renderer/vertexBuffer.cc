#ifdef __switch__
#include <string.h>
#else
#include <glad/gl.h>
#endif

#include "../util/align.h"
#include "../engine/engine.h"
#include "vertexBuffer.h"
#include "window.h"

std::array<std::unordered_set<render::VertexBuffer*>, 2> render::VertexBuffer::OutOfDateBuffers = {
	std::unordered_set<render::VertexBuffer*>(),
	std::unordered_set<render::VertexBuffer*>(),
};

render::VertexBuffer::VertexBuffer(Window* window, std::string debugName) {
	this->window = window;
	this->debugName = debugName;
}

render::VertexBuffer::~VertexBuffer() {
	this->destroyBuffer();
	VertexBuffer::OutOfDateBuffers[0].erase(this);
	VertexBuffer::OutOfDateBuffers[1].erase(this);

	if(this->isDynamicDraw) {
		if(this->dynamicBuffers[0].buffer != nullptr) {
			this->dynamicBuffers[0].buffer->deallocate();
			this->dynamicBuffers[0].buffer = nullptr;
		}

		if(this->dynamicBuffers[1].buffer != nullptr) {
			this->dynamicBuffers[1].buffer->deallocate();
			this->dynamicBuffers[1].buffer = nullptr;
		}
	}
	else {
		if(this->stagingBuffer != nullptr) {
			this->stagingBuffer->deallocate();
			this->stagingBuffer = nullptr;
		}
		
		if(this->gpuBuffer != nullptr) {
			this->gpuBuffer->deallocate();
			this->gpuBuffer = nullptr;
		}
	}
}

void render::VertexBuffer::reallocate() {
	#ifdef __switch__
	this->forceReallocate = true;
	#endif
}

#ifndef __switch__
vk::Buffer render::VertexBuffer::getVulkanBuffer() {
	return this->gpuBuffer->getBuffer();
}

vk::Buffer render::VertexBuffer::getVulkanDynamicBuffer() {
	return this->dynamicBuffers[this->window->getFramePingPong()].buffer->getBuffer();
}
#endif

void render::VertexBuffer::setDynamicDraw(bool isDynamicDraw) {
	#ifndef __switch__
	this->usage = isDynamicDraw ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
	this->isDynamicDraw = isDynamicDraw;
	this->oldSize = (uint32_t)-1; // force buffer reallocation
	#endif
}

// TODO port this over to double buffer system
void render::VertexBuffer::setSubData(void* data, unsigned int size, unsigned int offset) {
	if(this->size < size) {
		console::error("vertex data must be subset of data in setSubData\n");
		return;
	}

	#ifdef __switch__
	if(this->memory == nullptr) {
		console::error("vertex data must be initialized in setSubData\n");
		return;
	}
	
	memcpy((void*)((uintptr_t)this->memory->cpuAddr() + offset), data, size);
	#else
	if(this->window->backend == OPENGL_BACKEND) {
		if(this->bufferId == GL_INVALID_INDEX) {
			console::error("vertex data must be initialized in setSubData\n");
			return;
		}
		
		glBindBuffer(GL_ARRAY_BUFFER, this->bufferId);
		glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);

		#ifdef RENDER_UNBIND_VERTEX_BUFFERS
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		#endif
	}
	else {
		if(this->isDynamicDraw) {
			memcpy((void*)((uint64_t)this->gpuBuffer->map() + offset), data, size);
		}
		else {
			memcpy((void*)((uint64_t)this->stagingBuffer->map() + offset), data, size);
			this->needsCopy = true;
		}
	}
	#endif
}

void render::VertexBuffer::setData(void* data, unsigned int size, unsigned int align) {
	this->size = size;

	if(this->size == 0) {
		return;
	}
	
	#ifdef __switch__
	this->align = align;
	
	if(!this->memoryAllocated || alignTo(size, align) != this->memory->size() || this->forceReallocate) {
		this->destroyBuffer();
		this->allocateBuffer();
	}

	if(data != nullptr) {
		memcpy(this->memory->cpuAddr(), data, this->size);
	}
	this->memoryAllocated = true;
	#else
	if(this->window->backend == OPENGL_BACKEND) {
		if(this->bufferId == GL_INVALID_INDEX) {
			this->destroyBuffer();
			this->allocateBuffer();
		}

		if(this->usage == GL_DYNAMIC_DRAW) {
			glBindBuffer(GL_ARRAY_BUFFER, this->bufferId);
			glBufferData(GL_ARRAY_BUFFER, this->size, NULL, this->usage); // orphan the buffer
			glBufferSubData(GL_ARRAY_BUFFER, 0, this->size, data);

			#ifdef RENDER_UNBIND_VERTEX_BUFFERS
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			#endif
		}
		else {
			glBindBuffer(GL_ARRAY_BUFFER, this->bufferId);
			glBufferData(GL_ARRAY_BUFFER, this->size, data, this->usage);

			#ifdef RENDER_UNBIND_VERTEX_BUFFERS
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			#endif
		}
	}
	else {
		if(this->isDynamicDraw) {
			VertexSubBuffer &writeSubBuffer = this->dynamicBuffers[this->window->framePingPong];
			VertexSubBuffer &readSubBuffer = this->dynamicBuffers[!this->window->framePingPong];

			if(writeSubBuffer.buffer == nullptr || writeSubBuffer.buffer->getBufferSize() != this->size) {
				this->destroyBuffer();
				this->allocateBuffer();
			}
			
			if(data == nullptr) {
				memset(writeSubBuffer.buffer->map(), 0, this->size);
			}
			else {
				memcpy(writeSubBuffer.buffer->map(), data, this->size);
			}
			
			writeSubBuffer.state = VERTEX_SUB_BUFFER_IN_USE;

			// determine how the in-use buffer turns out of date
			if(readSubBuffer.state == VERTEX_SUB_BUFFER_IN_USE) {
				if(this->size != this->oldSize) {
					readSubBuffer.state = VERTEX_SUB_BUFFER_SIZE_OUT_OF_DATE;
				}
				else {
					readSubBuffer.state = VERTEX_SUB_BUFFER_DATA_OUT_OF_DATE;
				}
			}
			VertexBuffer::OutOfDateBuffers[!this->window->framePingPong].insert(this);
			VertexBuffer::OutOfDateBuffers[this->window->framePingPong].erase(this);
		}
		else {
			if(this->size != this->oldSize) {
				this->destroyBuffer();
				this->allocateBuffer();
			}
			
			if(data == nullptr) {
				memset(this->stagingBuffer->map(), 0, this->size);
			}
			else {
				memcpy(this->stagingBuffer->map(), data, this->size);
			}
			this->needsCopy = true;
		}
	}
	#endif

	this->oldSize = this->size;
}

#ifdef __switch__
void render::VertexBuffer::bind(unsigned short id) {
	this->window->commandBuffer.bindVtxBuffer(id, this->memory->gpuAddr(), this->memory->size());
}
#else
void render::VertexBuffer::bind() {
	glBindBuffer(GL_ARRAY_BUFFER, this->bufferId);
}
#endif

void render::VertexBuffer::allocateBuffer() {
	#ifdef __switch__
	this->memory = this->window->memory.allocate(DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached, this->size, this->align);
	#else
	if(this->window->backend == OPENGL_BACKEND) {
		if(this->usage == GL_DYNAMIC_DRAW) {
			this->usage = GL_DYNAMIC_DRAW;
			glGenBuffers(1, &this->bufferId);
			glBindBuffer(GL_ARRAY_BUFFER, this->bufferId);
			glBufferData(GL_ARRAY_BUFFER, this->size, NULL, this->usage);

			#ifdef RENDER_UNBIND_VERTEX_BUFFERS
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			#endif
		}
		else {
			this->usage = GL_STATIC_DRAW;
			glGenBuffers(1, &this->bufferId);
		}
	}
	else {
		if(this->isDynamicDraw) { // do not use staging buffer if we're dynamic draw
			this->dynamicBuffers[this->window->framePingPong].buffer = this->window->memory.allocateBuffer(
				#ifdef EGGINE_DEBUG
				this->debugName + "_dynamicVertexBuffer_" + std::to_string(this->window->framePingPong),
				#endif
				vk::BufferCreateInfo(
					{},
					size,
					vk::BufferUsageFlagBits::eVertexBuffer,
					vk::SharingMode::eExclusive
				),
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
			);

			if(!this->dynamicBufferInitialized) {
				this->dynamicBuffers[!this->window->framePingPong].buffer = this->window->memory.allocateBuffer(
					#ifdef EGGINE_DEBUG
					this->debugName + "_dynamicVertexBuffer_" + std::to_string(this->window->framePingPong),
					#endif
					vk::BufferCreateInfo(
						{},
						size,
						vk::BufferUsageFlagBits::eVertexBuffer,
						vk::SharingMode::eExclusive
					),
					vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
				);
				this->dynamicBufferInitialized = true;
			}
		}
		else {
			this->stagingBuffer = this->window->memory.allocateBuffer(
				#ifdef EGGINE_DEBUG
				this->debugName + "_vertexStagingBuffer",
				#endif
				vk::BufferCreateInfo(
					{},
					size,
					vk::BufferUsageFlagBits::eTransferSrc,
					vk::SharingMode::eExclusive
				),
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
			);
			
			this->gpuBuffer = this->window->memory.allocateBuffer(
				#ifdef EGGINE_DEBUG
				this->debugName + "_vertexGPUBuffer",
				#endif
				vk::BufferCreateInfo(
					{},
					size,
					vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
					vk::SharingMode::eExclusive
				),
				vk::MemoryPropertyFlagBits::eDeviceLocal
			);
		}
	}
	#endif
}

void render::VertexBuffer::destroyBuffer() {
	#ifdef __switch__
	if(this->memory != nullptr) {
		this->memory->deallocate();
	}
	this->memoryAllocated = false;
	#else
	if(this->window->backend == OPENGL_BACKEND) {
		if(this->bufferId != GL_INVALID_INDEX) {
			glDeleteBuffers(1, &this->bufferId);
		}
	}
	else {
		if(this->oldSize != 0) {
			if(this->stagingBuffer != nullptr) {
				this->stagingBuffer->deallocate();
				this->stagingBuffer = nullptr;
			}
			
			if(this->gpuBuffer != nullptr) {
				this->gpuBuffer->deallocate();
				this->gpuBuffer = nullptr;
			}

			if(this->dynamicBuffers[this->window->framePingPong].buffer != nullptr) {
				this->dynamicBuffers[this->window->framePingPong].buffer->deallocate();
				this->dynamicBuffers[this->window->framePingPong].buffer = nullptr;
			}
		}
	}
	#endif
}

void render::VertexBuffer::handleOutOfDateBuffer() {
	#ifdef __switch__
	#else
	if(!this->dynamicBufferInitialized) {
		return;
	}
	
	VertexSubBuffer &writeSubBuffer = this->dynamicBuffers[this->window->framePingPong];
	VertexSubBuffer &readSubBuffer = this->dynamicBuffers[!this->window->framePingPong];

	if(writeSubBuffer.state == VERTEX_SUB_BUFFER_SIZE_OUT_OF_DATE) {
		writeSubBuffer.buffer->deallocate();
		writeSubBuffer.buffer = this->window->memory.allocateBuffer(
			#ifdef EGGINE_DEBUG
			this->debugName + "_dynamicVertexBuffer_" + std::to_string(this->window->framePingPong),
			#endif
			vk::BufferCreateInfo(
				{},
				this->size,
				vk::BufferUsageFlagBits::eVertexBuffer,
				vk::SharingMode::eExclusive
			),
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		);
		memcpy(writeSubBuffer.buffer->map(), readSubBuffer.buffer->map(), this->size);
	}
	else if(writeSubBuffer.state == VERTEX_SUB_BUFFER_DATA_OUT_OF_DATE) {
		memcpy(writeSubBuffer.buffer->map(), readSubBuffer.buffer->map(), this->size);
	}

	writeSubBuffer.state = VERTEX_SUB_BUFFER_IN_USE;
	#endif
}

bool render::VertexBuffer::isOutOfDateBuffer() {
	#ifdef __switch__
	return false;
	#else
	VertexSubBuffer &writeSubBuffer = this->dynamicBuffers[this->window->framePingPong];
	return writeSubBuffer.state == VERTEX_SUB_BUFFER_SIZE_OUT_OF_DATE || writeSubBuffer.state == VERTEX_SUB_BUFFER_DATA_OUT_OF_DATE;
	#endif
}

void render::VertexBuffer::RemoveOutOfDateBuffer(render::VertexBuffer* buffer) {
	VertexBuffer::OutOfDateBuffers[engine->renderWindow.getFramePingPong()].erase(buffer);
}
