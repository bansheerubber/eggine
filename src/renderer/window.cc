#ifndef __switch__
#include <glad/gl.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../util/align.h"
#include "../engine/debug.h"
#include "../engine/engine.h"
#include "../resources/html.h"
#include "texture.h"
#include "window.h"

#ifndef __switch__
void onWindowResize(GLFWwindow* window, int width, int height) {
	engine->renderWindow.resize(width, height);
}
#endif

// opengl: initialize GLFW, glEnables, etc
// deko3d: create framebuffers/swapchains, command buffers
void render::Window::initialize() {
	#ifdef __switch__ // start of switch code (based on switch-examples/graphics/deko3d/deko_basic)
	this->device = dk::DeviceMaker{}.create();
	this->queue = dk::QueueMaker{this->device}.setFlags(DkQueueFlags_Graphics).create();

	// start the construction of our framebuffers
	dk::ImageLayout framebufferLayout;
	dk::ImageLayoutMaker{this->device}
		.setFlags(DkImageFlags_UsageRender | DkImageFlags_UsagePresent | DkImageFlags_HwCompression)
		.setFormat(DkImageFormat_RGBA8_Unorm)
		.setDimensions(this->width, this->height)
		.initialize(framebufferLayout); // helper data structure for framebuffer creation

	uint32_t framebufferSize  = framebufferLayout.getSize();
	uint32_t framebufferAlign = framebufferLayout.getAlignment();
	framebufferSize = alignTo(framebufferSize, framebufferAlign);

	// create the framebuffer's memory blocks from calculated size
	this->framebufferMemory = dk::MemBlockMaker{this->device, 2 * framebufferSize}.setFlags(DkMemBlockFlags_GpuCached | DkMemBlockFlags_Image).create();

	// create images for framebuffers
	for(unsigned int i = 0; i < 2; i++) {
		this->framebuffers[i].initialize(framebufferLayout, this->framebufferMemory, i * framebufferSize);
	}

	// create the swapchain using the framebuffers
	std::array<DkImage const*, 2> framebufferArray = { &this->framebuffers[0], &this->framebuffers[1] };
	this->swapchain = dk::SwapchainMaker{this->device, nwindowGetDefault(), framebufferArray}.create();

	// create the memory that we'll use for the command buffer
	this->staticCommandBufferMemory = dk::MemBlockMaker{this->device, this->staticCommandBufferSize}.setFlags(DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached).create();

	// create a buffer object for the command buffer
	this->staticCommandBuffer = dk::CmdBufMaker{this->device}.create();
	this->staticCommandBuffer.addMemory(this->staticCommandBufferMemory, 0, this->staticCommandBufferSize);

	// create command lists for our framebuffers, and also clarify them as render targets
	for(unsigned int i = 0; i < 2; i++) {
		dk::ImageView renderTarget { this->framebuffers[i] };
		this->staticCommandBuffer.bindRenderTargets(&renderTarget);
		this->framebufferCommandLists[i] = this->staticCommandBuffer.finishList();
	}

	this->blendState.setSrcColorBlendFactor(DkBlendFactor_SrcAlpha);
	this->blendState.setDstColorBlendFactor(DkBlendFactor_InvSrcAlpha);
	this->blendState.setSrcAlphaBlendFactor(DkBlendFactor_SrcAlpha);
	this->blendState.setDstAlphaBlendFactor(DkBlendFactor_InvSrcAlpha);

	this->colorState.setBlendEnable(0, true);

	// tell the switch that its time to disco
	this->staticCommandBuffer.setViewports(0, { this->viewport });
	this->staticCommandBuffer.setScissors(0, { this->scissor });
	this->staticCommandBuffer.clearColor(0, DkColorMask_RGBA, this->clearColor.r, this->clearColor.g, this->clearColor.b, this->clearColor.a);
	this->staticCommandBuffer.bindRasterizerState(this->rasterizerState);
	this->staticCommandBuffer.bindColorState(this->colorState);
	this->staticCommandBuffer.bindColorWriteState(this->colorWriteState);
	this->staticCommandBuffer.bindBlendStates(0, this->blendState);
	this->staticCommandList = this->staticCommandBuffer.finishList();

	// create the dynamic command buffer
	// create the memory that we'll use for the command buffer
	this->commandBufferMemory = dk::MemBlockMaker{this->device, this->commandBufferSize}.setFlags(DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached).create();

	// create a buffer object for the command buffer
	this->commandBuffer = dk::CmdBufMaker{this->device}.create();
	this->commandBuffer.addMemory(this->commandBufferMemory, this->commandBufferSliceSize * this->currentCommandBuffer, this->commandBufferSliceSize);

	// create a buffer object for the texture command buffer
	this->textureCommandBufferMemory = dk::MemBlockMaker{this->device, 4 * 1024}.setFlags(DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached).create();
	this->textureCommandBuffer = dk::CmdBufMaker{this->device}.create();
	this->textureCommandBuffer.addMemory(this->textureCommandBufferMemory, 0, 4 * 1024);

	// create image/sampler descriptor memory
	this->imageDescriptorMemory = this->memory.allocate(DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached, sizeof(DkImageDescriptor) * IMAGE_SAMPLER_DESCRIPTOR_COUNT, DK_IMAGE_DESCRIPTOR_ALIGNMENT);

	this->samplerDescriptorMemory = this->memory.allocate(DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached, sizeof(DkSamplerDescriptor) * IMAGE_SAMPLER_DESCRIPTOR_COUNT, DK_SAMPLER_DESCRIPTOR_ALIGNMENT);

	// initialize gamepad
	padConfigureInput(1, HidNpadStyleSet_NpadStandard);
	padInitializeDefault(&this->pad);
	#else // else for ifdef __switch__
	if(!glfwInit()) {
		printf("failed to initialize glfw\n");
		exit(1);
	}

	// support 4.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	this->window = glfwCreateWindow(this->width, this->height, "eggine", NULL, NULL);
	glfwMakeContextCurrent(window);
	gladLoadGL(glfwGetProcAddress);

	glfwSetWindowSizeCallback(this->window, onWindowResize);

	glEnable(GL_BLEND);
	// glEnable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glfwSwapInterval(1);

	#ifdef EGGINE_DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
	glDebugMessageCallback(glDebugOutput, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	#endif

	#ifdef EGGINE_DEVELOPER_MODE
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(this->window, true);
	ImGui_ImplOpenGL3_Init("#version 150");
	#endif

	#endif // end for ifdef __switch__

	this->htmlContainer = new LiteHTMLContainer();

	resources::HTML* html = (resources::HTML*)engine->manager->loadResources(engine->manager->carton->database.get()->equals("fileName", "html/index.html")->exec())[0];
	engine->manager->loadResources(engine->manager->carton->database.get()->equals("extension", ".css")->exec());
	this->htmlDocument = litehtml::document::createFromString(html->document.c_str(), this->htmlContainer, &this->htmlContext);
}

void render::Window::addError() {
	this->errorCount++;
}

unsigned int render::Window::getErrorCount() {
	return this->errorCount;
}

void render::Window::clearErrors() {
	this->errorCount = 0;
}

void render::Window::registerHTMLUpdate() {
	this->htmlChecksum++;
}

void render::Window::deinitialize() {
	#ifdef __switch__
	this->queue.waitIdle();

	this->queue.destroy();
	this->staticCommandBuffer.destroy();
	this->staticCommandBufferMemory.destroy();
	this->commandBuffer.destroy();
	this->commandBufferMemory.destroy();
	this->swapchain.destroy();
	this->framebufferMemory.destroy();
	this->device.destroy();
	#else
	glfwTerminate();
	#endif
}

void render::Window::prerender() {
	long long startTime = getMicrosecondsNow();
	this->deltaTime = (startTime - this->lastRenderTime) / 1000000.0;
	this->lastRenderTime = getMicrosecondsNow();
	
	#ifdef __switch__
	// do dynamic command buffer magic
	this->commandBuffer.clear();
	this->commandBufferFences[this->signaledFence].wait();
	this->commandBuffer.addMemory(this->commandBufferMemory, this->commandBufferSliceSize * this->currentCommandBuffer, this->commandBufferSliceSize);

	// handle deallocated memory at the beginning of each frame
	this->memory.processDeallocationLists();

	// handle gamepad
	padUpdate(&this->pad);

	this->leftStick = padGetStickPos(&this->pad, 0);
	this->rightStick = padGetStickPos(&this->pad, 1);
	this->buttons = padGetButtons(&this->pad);
	#else
	glClearColor(this->clearColor.r, this->clearColor.g, this->clearColor.b, this->clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glfwPollEvents();
	this->hasGamepad = glfwGetGamepadState(GLFW_JOYSTICK_1, &this->gamepad);
	#endif
}

void render::Window::render() {
	litehtml::position clip;
	this->htmlContainer->get_client_rect(clip);
	
	if(this->htmlChecksum != this->lastHTMLChecksum) {
		this->htmlDocument->render(clip.width);
		this->lastHTMLChecksum = this->htmlChecksum;
	}
	
	this->htmlDocument->draw(0, 0, 0, nullptr);
	
	#ifdef __switch__
	int index = this->queue.acquireImage(this->swapchain);
	this->queue.submitCommands(this->framebufferCommandLists[index]);
	this->queue.submitCommands(this->staticCommandList);

	// do dynamic command buffer magic
	this->commandBuffer.signalFence(this->commandBufferFences[this->currentCommandBuffer]);
	this->signaledFence = this->currentCommandBuffer;
	this->queue.submitCommands(this->commandBuffer.finishList());

	this->currentCommandBuffer = (this->currentCommandBuffer + 1) % this->commandBufferCount;

	this->queue.presentImage(this->swapchain, index);
	#else
	glfwSwapBuffers(this->window);
	#endif
}

void render::Window::draw(PrimitiveType type, unsigned int firstVertex, unsigned int vertexCount, unsigned int firstInstance, unsigned int instanceCount) {
	#ifdef __switch__
	this->commandBuffer.draw(primitiveToDkPrimitive(type), vertexCount, instanceCount, firstVertex, firstInstance);
	#else
	if(firstInstance == 0 && instanceCount == 1) {
		glDrawArrays(primitiveToGLPrimitive(type), firstVertex, vertexCount);
	}
	else {
		glDrawArraysInstancedBaseInstance(primitiveToGLPrimitive(type), firstVertex, vertexCount, instanceCount, firstInstance);
	}
	#endif
}

void render::Window::resize(unsigned int width, unsigned int height) {
	this->registerHTMLUpdate();
	this->width = width;
	this->height = height;
	
	#ifndef __switch__
	glViewport(0, 0, width, height);
	#endif
}

#ifdef __switch__
void render::Window::addTexture(switch_memory::Piece* tempMemory, dk::ImageView& view, unsigned int width, unsigned int height) {
	this->queue.waitIdle();
	this->textureCommandBuffer.clear();
	this->textureCommandBuffer.addMemory(this->textureCommandBufferMemory, 0, 4 * 1024);

	this->textureCommandBuffer.copyBufferToImage({ tempMemory->gpuAddr() }, view, { 0, 0, 0, width, height, 1 });
	this->queue.submitCommands(this->textureCommandBuffer.finishList());
	this->queue.waitIdle(); // wait to add the texture
	this->textureCommandBuffer.clear();
}

void render::Window::bindTexture(unsigned int location, Texture* texture) {
	this->commandBuffer.pushData(this->imageDescriptorMemory->gpuAddr() + location * sizeof(DkImageDescriptor), &texture->imageDescriptor, sizeof(DkImageDescriptor));
	this->commandBuffer.pushData(this->samplerDescriptorMemory->gpuAddr() + location * sizeof(DkSamplerDescriptor), &texture->samplerDescriptor, sizeof(DkSamplerDescriptor));

	this->commandBuffer.bindImageDescriptorSet(this->imageDescriptorMemory->gpuAddr(), IMAGE_SAMPLER_DESCRIPTOR_COUNT);
	this->commandBuffer.bindSamplerDescriptorSet(this->samplerDescriptorMemory->gpuAddr(), IMAGE_SAMPLER_DESCRIPTOR_COUNT);
}
#endif
