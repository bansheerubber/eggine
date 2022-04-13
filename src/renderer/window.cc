#ifndef __switch__
#include <glad/gl.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../util/align.h"
#include "../engine/console.h"
#include "../engine/debug.h"
#include "debug.h"
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

	this->rasterizerState.setCullMode(DkFace_None);

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
		console::error("failed to initialize glfw\n");
		exit(1);
	}

	// support 4.3
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	this->window = glfwCreateWindow(this->width, this->height, "eggine", NULL, NULL);
	// glfwMakeContextCurrent(window);
	glfwSetErrorCallback(glfwErrorCallback);
	// gladLoadGL(glfwGetProcAddress);

	// glfwSetWindowSizeCallback(this->window, onWindowResize);

	// glEnable(GL_BLEND);
	// this->enableDepthTest(true);
	// this->enableStencilTest(true);
	// // glEnable(GL_CULL_FACE);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// glfwSwapInterval(1);

	// #ifdef EGGINE_DEBUG
	// glEnable(GL_DEBUG_OUTPUT);
	// glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
	// glDebugMessageCallback(glDebugOutput, nullptr);
	// glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	// #endif

	// #ifdef EGGINE_DEVELOPER_MODE
	// IMGUI_CHECKVERSION();
	// ImGui::CreateContext();
	// ImGuiIO& io = ImGui::GetIO(); (void)io;
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	// ImGui::StyleColorsDark();
	// ImGui_ImplGlfw_InitForOpenGL(this->window, false);
	// ImGui_ImplOpenGL3_Init("#version 150");
	// #endif

	VkApplicationInfo app {
		sType: VK_STRUCTURE_TYPE_APPLICATION_INFO,
		pApplicationName: "VulkanClear",
		applicationVersion: VK_MAKE_VERSION(1, 0, 0),
		pEngineName: "ClearScreenEngine",
		engineVersion: VK_MAKE_VERSION(1, 0, 0),
		apiVersion: VK_API_VERSION_1_3,
	};

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount); // TODO turn this into std::string so we can add our own extensions?

	std::vector<const char*> extensions;
	for(uint32_t i = 0; i < glfwExtensionCount; i++) {
		extensions.push_back(glfwExtensions[i]);
	}

	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	extensions.push_back("VK_KHR_xlib_surface");
	extensions.push_back("VK_KHR_display");

	VkInstanceCreateInfo createInfo {
		sType: VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		pApplicationInfo: &app,
		enabledLayerCount: (uint32_t)RequiredValidationLayers.size(),
		ppEnabledLayerNames: RequiredValidationLayers.data(),
		enabledExtensionCount: (uint32_t)extensions.size(),
		ppEnabledExtensionNames: extensions.data(),
	};

	VkResult result = vkCreateInstance(&createInfo, nullptr, &this->instance);
	if(result != VK_SUCCESS) {
		console::error("vulkan: could not create instance: %s\n", vkResultToString(result).c_str());
		exit(1);
	}

	// handle debug
	VkDebugUtilsMessengerCreateInfoEXT debugInfo = {
		sType: VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		messageSeverity: VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
		messageType: VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		pfnUserCallback: vulkanDebugCallback,
	};

	PFN_vkCreateDebugUtilsMessengerEXT CreateDebugReportCallback
		= (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(this->instance, "vkCreateDebugUtilsMessengerEXT");

	if(CreateDebugReportCallback == nullptr) {
		console::error("vulkan: could not find vkCreateDebugUtilsMessengerEXT\n");
		exit(1);
	}

	result = CreateDebugReportCallback(this->instance, &debugInfo, nullptr, &this->debugCallback);
	if(result != VK_SUCCESS) {
		console::error("vulkan: could not create debug callback: %s\n", vkResultToString(result).c_str());
		exit(1);
	}

	// handle surface
	result = glfwCreateWindowSurface(this->instance, this->window, nullptr, &this->surface);
	if(result != VK_SUCCESS || this->surface == VK_NULL_HANDLE) {
		console::error("vulkan: could not create window surface: %s\n", vkResultToString(result).c_str());
		exit(1);
	}

	// handle physical devices
	this->pickDevice();
	this->setupDevice();

	console::print("yippee\n");

	exit(0);
	#endif // end for ifdef __switch__
}

void render::Window::initializeHTML() {
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

void render::Window::setStencilFunction(render::StencilFunction func, unsigned int reference, unsigned int mask) {
	#ifdef __switch__
	#else
	glStencilFunc(stencilToGLStencil(func), reference, mask);
	#endif
}

void render::Window::setStencilMask(unsigned int mask) {
	#ifdef __switch__
	#else
	glStencilMask(mask);
	#endif
}

void render::Window::setStencilOperation(StencilOperation stencilFail, StencilOperation depthFail, StencilOperation pass) {
	#ifdef __switch__
	#else
	glStencilOp(stencilOPToGLStencilOP(stencilFail), stencilOPToGLStencilOP(depthFail), stencilOPToGLStencilOP(pass));
	#endif
}

void render::Window::enableStencilTest(bool enable) {
	#ifdef __switch__
	#else
	if(enable) {
		glEnable(GL_STENCIL_TEST);
	}
	else {
		glDisable(GL_STENCIL_TEST);
	}
	#endif
}

void render::Window::enableDepthTest(bool enable) {
	#ifdef __switch__
	#else
	if(enable) {
		glEnable(GL_DEPTH_TEST);
	}
	else {
		glDisable(GL_DEPTH_TEST);
	}
	#endif
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
	vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
	vkDestroyInstance(this->instance, nullptr);

	for(auto renderImageView: this->renderImageViews) {
		vkDestroyImageView(this->device.device, renderImageView, nullptr);
	}

	vkDestroySwapchainKHR(this->device.device, this->swapchain, nullptr);
	vkDestroyDevice(this->device.device, nullptr);
	glfwTerminate();
	#endif
}

void render::Window::prerender() {
	int64_t startTime = getMicrosecondsNow();
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
	glStencilMask(0xFF);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
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
	if(glfwWindowShouldClose(this->window)) {
		engine->exit();
	}
	
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
#else
void render::Window::pickDevice() {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(this->instance, &deviceCount, nullptr);
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(this->instance, &deviceCount, devices.data());

	Device integratedGPU = { VK_NULL_HANDLE, VK_NULL_HANDLE };
	Device discreteGPU = { VK_NULL_HANDLE, VK_NULL_HANDLE };
	for(VkPhysicalDevice device: devices) {
		Device potentialDevice = {
			VK_NULL_HANDLE, device, {}, {}, (uint32_t)-1, (uint32_t)-1,
		};
		
		vkGetPhysicalDeviceProperties(device, &potentialDevice.properties); // devices properties (name, etc)
		vkGetPhysicalDeviceFeatures(device, &potentialDevice.features); // device features (texture sizes, supported shaders, etc)

		// handle searching for queues
		{
			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

			for(size_t i = 0; i < queueFamilies.size(); i++) {
				if((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && potentialDevice.graphicsQueueIndex == (uint32_t)-1) {
					potentialDevice.graphicsQueueIndex = i;
				}

				bool presentSupport = glfwGetPhysicalDevicePresentationSupport(this->instance, device, i);
				if(presentSupport && potentialDevice.presentationQueueIndex == (uint32_t)-1) {
					potentialDevice.presentationQueueIndex = i;
				}
			}

			if(potentialDevice.graphicsQueueIndex == (uint32_t)-1 || potentialDevice.presentationQueueIndex == (uint32_t)-1) {
				continue;
			}
		}

		// handle required extensions
		{
			uint32_t extensionCount;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

			std::vector<VkExtensionProperties> availableExtensions(extensionCount);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

			uint32_t foundExtensions = 0;
			for(auto &extension: availableExtensions) {
				if(std::find_if(
					RequiredDeviceExtensions.begin(),
					RequiredDeviceExtensions.end(),
					[extension] (const char* s) { return std::string(s) == std::string(extension.extensionName); }
				) != RequiredDeviceExtensions.end()) {
					foundExtensions++;
				}
			}

			if(foundExtensions != RequiredDeviceExtensions.size()) {
				continue;
			}
		}

		// handle swapchain support
		{
			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(potentialDevice.physicalDevice, this->surface, &formatCount, nullptr);

			if(formatCount != 0) {
				potentialDevice.surfaceFormats.resize(formatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(potentialDevice.physicalDevice, this->surface, &formatCount, potentialDevice.surfaceFormats.data());
			}
			else {
				continue;
			}

			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(potentialDevice.physicalDevice, this->surface, &presentModeCount, nullptr);

			if(presentModeCount != 0) {
				potentialDevice.presentModes.resize(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(potentialDevice.physicalDevice, this->surface, &presentModeCount, potentialDevice.presentModes.data());
			}
			else {
				continue;
			}

			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(potentialDevice.physicalDevice, this->surface, &potentialDevice.capabilities);
		}

		// finally, we are done
		if(potentialDevice.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU || potentialDevice.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU) {
			integratedGPU = potentialDevice;
		}
		else if(potentialDevice.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || potentialDevice.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU) {
			discreteGPU = potentialDevice;
		}
	}

	Device selectedDevice = discreteGPU.physicalDevice != VK_NULL_HANDLE ? discreteGPU : integratedGPU;
	if(selectedDevice.physicalDevice == VK_NULL_HANDLE) {
		console::print("vulkan: could not find suitable display device\n");
		exit(1);
	}

	console::print("vulkan: selected device '%s'\n", selectedDevice.properties.deviceName);
	this->device = selectedDevice;
}

void render::Window::setupDevice() {
	float queuePriority = 1.0f;
	std::set<uint32_t> queues = { this->device.graphicsQueueIndex, this->device.presentationQueueIndex };
	std::vector<VkDeviceQueueCreateInfo> creationInfos;
	for(auto queue: queues) {
		creationInfos.push_back(VkDeviceQueueCreateInfo {
			sType: VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			queueFamilyIndex: queue,
			queueCount: 1,
			pQueuePriorities: &queuePriority,
		});
	}

	VkPhysicalDeviceFeatures deviceFeatures {};
	VkDeviceCreateInfo deviceCreateInfo {
		sType: VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		queueCreateInfoCount: (uint32_t)creationInfos.size(),
		pQueueCreateInfos: creationInfos.data(),
		enabledLayerCount: 0,
		enabledExtensionCount: (uint32_t)RequiredDeviceExtensions.size(),
		ppEnabledExtensionNames: RequiredDeviceExtensions.data(),
		pEnabledFeatures: &deviceFeatures,
	};
	
	VkResult result = vkCreateDevice(this->device.physicalDevice, &deviceCreateInfo, nullptr, &this->device.device);
	if(result != VK_SUCCESS) {
		console::print("vulkan: could not create device with reason '%s'\n", vkResultToString(result).c_str());
		exit(1);
	}

	vkGetDeviceQueue(this->device.device, this->device.graphicsQueueIndex, 0, &this->queue);
	vkGetDeviceQueue(this->device.device, this->device.presentationQueueIndex, 0, &this->presentationQueue);

	// now create the swapchain
	VkSurfaceFormatKHR format = { VK_FORMAT_UNDEFINED, };
	for(VkSurfaceFormatKHR f: this->device.surfaceFormats) {
		if(f.format == VK_FORMAT_B8G8R8A8_SRGB && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			format = f;
		}
	}

	if(format.format == VK_FORMAT_UNDEFINED) {
		console::print("vulkan: could not find suitable surface format\n");
		exit(1);
	}

	uint32_t width, height;
	glfwGetFramebufferSize(this->window, (int*)&width, (int*)&height);

	width = std::clamp(width, this->device.capabilities.minImageExtent.width, this->device.capabilities.maxImageExtent.width);
	height = std::clamp(width, this->device.capabilities.minImageExtent.height, this->device.capabilities.maxImageExtent.height);

	uint32_t imageCount = 2; // TODO this needs to be double checked against the drivers
	VkSwapchainCreateInfoKHR swapChainInfo {
		sType: VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		surface: this->surface,
		minImageCount: imageCount,
		imageFormat: format.format,
		imageColorSpace: format.colorSpace,
		imageExtent: VkExtent2D { width, height },
		imageArrayLayers: 1,
		imageUsage: VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		preTransform: this->device.capabilities.currentTransform,
		compositeAlpha: VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		presentMode: VK_PRESENT_MODE_FIFO_KHR,
		clipped: VK_TRUE,
		oldSwapchain: VK_NULL_HANDLE,
	};

	// handle multiple queues
	if(this->device.graphicsQueueIndex != this->device.presentationQueueIndex) {
		swapChainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapChainInfo.queueFamilyIndexCount = 2;
		swapChainInfo.pQueueFamilyIndices = &this->device.graphicsQueueIndex; // i dare you to crash vulkan
	}
	else {
		swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	result = vkCreateSwapchainKHR(this->device.device, &swapChainInfo, nullptr, &this->swapchain);
	if(result != VK_SUCCESS) {
		console::print("vulkan: could not create swapchain with reason '%s'\n", vkResultToString(result).c_str());
		exit(1);
	}

	vkGetSwapchainImagesKHR(this->device.device, this->swapchain, &imageCount, nullptr);
	this->renderImages.resize(imageCount);
	vkGetSwapchainImagesKHR(this->device.device, this->swapchain, &imageCount, this->renderImages.data());

	// create image views
	this->renderImageViews.resize(imageCount);
	for(uint32_t i = 0; i < imageCount; i++) {
		VkImageViewCreateInfo imageViewInfo {
			sType: VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			image: this->renderImages[i],
			viewType: VK_IMAGE_VIEW_TYPE_2D,
			format: format.format,
			components: { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, },
			subresourceRange: { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
		};

		result = vkCreateImageView(this->device.device, &imageViewInfo, nullptr, &this->renderImageViews[i]);
		if(result != VK_SUCCESS) {
			console::print("vulkan: could not create swapchain image view with reason '%s'\n", vkResultToString(result).c_str());
			exit(1);
		}
	}
}
#endif
