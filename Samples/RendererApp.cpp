#include "RendererApp.h"

void RendererApp::Load()
{
	windowManager.CreateSurface(deviceManager.instance);

	renderer.clearColor = { 0.f, 0.f, 0.2f, 1.f };
	renderer.Initialize(&deviceManager, windowManager.surface, width, height);

	meshManager.Initialize(&deviceManager, &renderer.graphicsQueue);

	sceneManager.Initialize(&meshManager);

	SceneCreateInfo createInfo;
	createInfo.meshesToLoad = { "defaultBox" };
	sceneManager.CreateScene(&createInfo);

	LoadVertexAndIndexBuffers();
	LoadShaders();

	CreateUniformBuffers();

	renderer.BuildPipelines(shaderStages);
	renderer.BuildDescriptorSets(&uniformBufferVS.descriptor);
	renderer.BuildDrawCommandBuffers(&vertices.buffer, &indices.buffer, indices.count);
}

void RendererApp::UnLoad()
{
	for (auto& shader : shaderStages)
		vkDestroyShaderModule(deviceManager.logicalDevice, shader.module, nullptr);

	vkDestroyBuffer(deviceManager.logicalDevice, vertices.buffer, nullptr);
	vkFreeMemory(deviceManager.logicalDevice, vertices.memory, nullptr);

	vkDestroyBuffer(deviceManager.logicalDevice, indices.buffer, nullptr);
	vkFreeMemory(deviceManager.logicalDevice, indices.memory, nullptr);

	vkDestroyBuffer(deviceManager.logicalDevice, uniformBufferVS.buffer, nullptr);
	vkFreeMemory(deviceManager.logicalDevice, uniformBufferVS.memory, nullptr);
}

void RendererApp::Update(__int64 time)
{
	auto t = time / 1000000.f;

	if (windowManager.window == nullptr)
		return;

	if (glfwGetKey(windowManager.window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		shouldShutDown = true;

	if (glfwGetKey(windowManager.window, GLFW_KEY_F1) == GLFW_PRESS)
		windowManager.SetWindowMode(WindowModes::Window, width, height);

	if (glfwGetKey(windowManager.window, GLFW_KEY_F2) == GLFW_PRESS)
		windowManager.SetWindowMode(WindowModes::Borderless, 1920, 1080);

	if (glfwGetKey(windowManager.window, GLFW_KEY_F3) == GLFW_PRESS)
		windowManager.SetWindowMode(WindowModes::Fullscreen, 1920, 1080);

	if (glfwGetMouseButton(windowManager.window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		position.z += 1.f * t;

	if (glfwGetMouseButton(windowManager.window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		position.z -= 1.f * t;

	if (glfwGetKey(windowManager.window, GLFW_KEY_LEFT) == GLFW_PRESS)
		modelPosition.x -= 1.f * t;

	if (glfwGetKey(windowManager.window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		modelPosition.x += 1.f * t;

	if (glfwGetKey(windowManager.window, GLFW_KEY_UP) == GLFW_PRESS)
		modelPosition.y -= 1.f * t;

	if (glfwGetKey(windowManager.window, GLFW_KEY_DOWN) == GLFW_PRESS)
		modelPosition.y += 1.f * t;

	UpdateUniformBuffers();
}

void RendererApp::DrawFrame()
{
	renderer.RenderFrame();
}

void RendererApp::WindowSizeChanged()
{
	vkDeviceWaitIdle(deviceManager.logicalDevice);

	renderer.ResizeResources(shaderStages, &uniformBufferVS.descriptor, &vertices.buffer, &indices.buffer, indices.count);
}

RendererApp::~RendererApp()
{
	UnLoad();
}

void RendererApp::LoadVertexAndIndexBuffers()
{
	// A note on memory management in Vulkan in general:
	//	This is a very complex topic and while it's fine for an example application to to small individual memory allocations that is not
	//	what should be done a real-world application, where you should allocate large chunkgs of memory at once isntead.

	struct Vertex
	{
		float position[3];
		float color[3];
	};

	/*
    z+     y-
	   \   |
	     \ |
	x- ----\----  x+
	   	   | \
	   	   |   \
		   y+     z-
	*/

	// Setup vertices
	std::vector<Vertex> vertexBuffer =
	{
		{ { 1.0f,  1.0f, 0.0f },{ 1.0f, 0.0f, 0.0f } },
		{ { -1.0f,  1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } },
		{ { -1.0f,  -1.0f, 0.0f },{ 0.0f, 1.0f, 1.0f } },
		{ { 1.0f, -1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } },

		{ { 1.0f,  1.0f, -1.0f },{ 0.0f, 0.0f, 1.0f } },
		{ { -1.0f,  1.0f, -1.0f },{ 0.0f, 1.0f, 1.0f } },
		{ { -1.0f,  -1.0f, -1.0f },{ 0.0f, 1.0f, 0.0f } },
		{ { 1.0f, -1.0f, -1.0f },{ 1.0f, 0.0f, 0.0f } }
	};
	uint32_t vertexBufferSize = static_cast<uint32_t>(vertexBuffer.size()) * sizeof(Vertex);

	// Setup indices
	std::vector<uint32_t> indexBuffer = { 
		// front
		0, 1, 2,
		2, 3, 0,
		// top
		2, 6, 7,
		7, 3, 2,
		// left
		1, 5, 6,
		6, 2, 1,
		// right
		4, 0, 3,
		3, 7, 4,
		// bottom
		0, 4, 5,
		5, 1, 0,
		// back
		4, 7, 5,
		5, 7, 6,
	};
	indices.count = static_cast<uint32_t>(indexBuffer.size());
	uint32_t indexBufferSize = indices.count * sizeof(uint32_t);

	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements memReqs;

	void *data;

	// Static data like vertex and index buffer should be stored on the device memory 
	// for optimal (and fastest) access by the GPU
	//
	// To achieve this we use so-called "staging buffers" :
	// - Create a buffer that's visible to the host (and can be mapped)
	// - Copy the data to this buffer
	// - Create another buffer that's local on the device (VRAM) with the same size
	// - Copy the data from the host to the device using a command buffer
	// - Delete the host visible (staging) buffer
	// - Use the device local buffers for rendering

	struct StagingBuffer {
		VkDeviceMemory memory;
		VkBuffer buffer;
	};

	struct {
		StagingBuffer vertices;
		StagingBuffer indices;
	} stagingBuffers;

	// Vertex buffer
	VkBufferCreateInfo vertexBufferInfo = {};
	vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexBufferInfo.size = vertexBufferSize;
	// Buffer is used as the copy source
	vertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	// Create a host-visible buffer to copy the vertex data to (staging buffer)
	if (vkCreateBuffer(deviceManager.logicalDevice, &vertexBufferInfo, nullptr, &stagingBuffers.vertices.buffer) != VK_SUCCESS)
		throw std::runtime_error("failed to create buffer");

	vkGetBufferMemoryRequirements(deviceManager.logicalDevice, stagingBuffers.vertices.buffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	// Request a host visible memory type that can be used to copy our data do
	// Also request it to be coherent, so that writes are visible to the GPU right after unmapping the buffer
	memAlloc.memoryTypeIndex = deviceManager.GetMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	if (vkAllocateMemory(deviceManager.logicalDevice, &memAlloc, nullptr, &stagingBuffers.vertices.memory) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate buffer");

	// Map and copy
	if (vkMapMemory(deviceManager.logicalDevice, stagingBuffers.vertices.memory, 0, memAlloc.allocationSize, 0, &data) != VK_SUCCESS)
		throw std::runtime_error("failed to map buffer");

	memcpy(data, vertexBuffer.data(), vertexBufferSize);
	vkUnmapMemory(deviceManager.logicalDevice, stagingBuffers.vertices.memory);
	if (vkBindBufferMemory(deviceManager.logicalDevice, stagingBuffers.vertices.buffer, stagingBuffers.vertices.memory, 0) != VK_SUCCESS)
		throw std::runtime_error("failed to bind buffer");

	// Create a device local buffer to which the (host local) vertex data will be copied and which will be used for rendering
	vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	if (vkCreateBuffer(deviceManager.logicalDevice, &vertexBufferInfo, nullptr, &vertices.buffer) != VK_SUCCESS)
		throw std::runtime_error("failed to create buffer");

	vkGetBufferMemoryRequirements(deviceManager.logicalDevice, vertices.buffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = deviceManager.GetMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if (vkAllocateMemory(deviceManager.logicalDevice, &memAlloc, nullptr, &vertices.memory) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate memory");
	if (vkBindBufferMemory(deviceManager.logicalDevice, vertices.buffer, vertices.memory, 0) != VK_SUCCESS)
		throw std::runtime_error("failed to bind memory");

	// Index buffer
	VkBufferCreateInfo indexbufferInfo = {};
	indexbufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	indexbufferInfo.size = indexBufferSize;
	indexbufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	// Copy index data to a buffer visible to the host (staging buffer)
	if (vkCreateBuffer(deviceManager.logicalDevice, &indexbufferInfo, nullptr, &stagingBuffers.indices.buffer) != VK_SUCCESS)
		throw std::runtime_error("failed to create buffer");

	vkGetBufferMemoryRequirements(deviceManager.logicalDevice, stagingBuffers.indices.buffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = deviceManager.GetMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	if (vkAllocateMemory(deviceManager.logicalDevice, &memAlloc, nullptr, &stagingBuffers.indices.memory) != VK_SUCCESS)
		throw std::runtime_error("failed to create buffer");
	if (vkMapMemory(deviceManager.logicalDevice, stagingBuffers.indices.memory, 0, indexBufferSize, 0, &data) != VK_SUCCESS)
		throw std::runtime_error("failed to create buffer");
	memcpy(data, indexBuffer.data(), indexBufferSize);
	vkUnmapMemory(deviceManager.logicalDevice, stagingBuffers.indices.memory);
	if (vkBindBufferMemory(deviceManager.logicalDevice, stagingBuffers.indices.buffer, stagingBuffers.indices.memory, 0) != VK_SUCCESS)
		throw std::runtime_error("failed to create buffer");

	// Create destination buffer with device only visibility
	indexbufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	if (vkCreateBuffer(deviceManager.logicalDevice, &indexbufferInfo, nullptr, &indices.buffer) != VK_SUCCESS)
		throw std::runtime_error("failed to create buffer");
	vkGetBufferMemoryRequirements(deviceManager.logicalDevice, indices.buffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = deviceManager.GetMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if (vkAllocateMemory(deviceManager.logicalDevice, &memAlloc, nullptr, &indices.memory) != VK_SUCCESS)
		throw std::runtime_error("failed to create buffer");
	if (vkBindBufferMemory(deviceManager.logicalDevice, indices.buffer, indices.memory, 0) != VK_SUCCESS)
		throw std::runtime_error("failed to create buffer");

	VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufferBeginInfo.pNext = nullptr;

	// Buffer copies have to be submitted to a queue, so we need a command buffer for them
	// Note: Some devices offer a dedicated transfer queue (with only the transfer bit set) that may be faster when doing lots of copies
	VkCommandBuffer copyCmd;

	VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
	cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufAllocateInfo.commandPool = deviceManager.commandPool;
	cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufAllocateInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(deviceManager.logicalDevice, &cmdBufAllocateInfo, &copyCmd) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate command buffer");

	VkCommandBufferBeginInfo cmdBufferBeginInfo2 = {};
	cmdBufferBeginInfo2.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufferBeginInfo2.pNext = NULL;

	if (vkBeginCommandBuffer(copyCmd, &cmdBufferBeginInfo2) != VK_SUCCESS)
		throw std::runtime_error("failed to BeginCommandBuffer");

	// Put buffer region copies into command buffer
	VkBufferCopy copyRegion = {};

	// Vertex buffer
	copyRegion.size = vertexBufferSize;
	vkCmdCopyBuffer(copyCmd, stagingBuffers.vertices.buffer, vertices.buffer, 1, &copyRegion);
	// Index buffer
	copyRegion.size = indexBufferSize;
	vkCmdCopyBuffer(copyCmd, stagingBuffers.indices.buffer, indices.buffer, 1, &copyRegion);

	// Flushing the command buffer will also submit it to the queue and uses a fence to ensure that all commands have been executed before returning
	if (vkEndCommandBuffer(copyCmd) != VK_SUCCESS)
		throw std::runtime_error("failed to EndCommandBuffer");

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &copyCmd;

	// Create fence to ensure that the command buffer has finished executing
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = 0;

	VkFence fence;
	if (vkCreateFence(deviceManager.logicalDevice, &fenceCreateInfo, nullptr, &fence) != VK_SUCCESS)
		throw std::runtime_error("failed to CreateFence");

	// Submit to the queue
	if (vkQueueSubmit(renderer.graphicsQueue, 1, &submitInfo, fence) != VK_SUCCESS)
		throw std::runtime_error("failed to QueueSubmit");

	// Wait for the fence to signal that command buffer has finished executing
	if (vkWaitForFences(deviceManager.logicalDevice, 1, &fence, VK_TRUE, 100000000000) != VK_SUCCESS)
		throw std::runtime_error("failed to WaitForFences");

	vkDestroyFence(deviceManager.logicalDevice, fence, nullptr);
	vkFreeCommandBuffers(deviceManager.logicalDevice, deviceManager.commandPool, 1, &copyCmd);

	// Destroy staging buffers
	// Note: Staging buffer must not be deleted before the copies have been submitted and executed
	vkDestroyBuffer(deviceManager.logicalDevice, stagingBuffers.vertices.buffer, nullptr);
	vkFreeMemory(deviceManager.logicalDevice, stagingBuffers.vertices.memory, nullptr);
	vkDestroyBuffer(deviceManager.logicalDevice, stagingBuffers.indices.buffer, nullptr);
	vkFreeMemory(deviceManager.logicalDevice, stagingBuffers.indices.memory, nullptr);
}

void RendererApp::LoadShaders()
{
	shaderStages.resize(2);

	// Vertex shader
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = shaderManager.CreateShaderModule(deviceManager.logicalDevice, "shaders/triangle.vert.spv");
	shaderStages[0].pName = "main";

	// Fragment shader
	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = shaderManager.CreateShaderModule(deviceManager.logicalDevice, "shaders/triangle.frag.spv");
	shaderStages[1].pName = "main";
}

void RendererApp::CreateUniformBuffers()
{
	// Prepare and initialize a uniform buffer block containing shader uniforms
	// Single uniforms like in OpenGL are no longer present in Vulkan. All Shader uniforms are passed via uniform buffer blocks
	VkMemoryRequirements memReqs;

	// Vertex shader uniform buffer block
	VkBufferCreateInfo bufferInfo = {};
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.allocationSize = 0;
	allocInfo.memoryTypeIndex = 0;

	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = sizeof(uboVS);
	// This buffer will be used as a uniform buffer
	bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	// Create a new buffer
	if (vkCreateBuffer(deviceManager.logicalDevice, &bufferInfo, nullptr, &uniformBufferVS.buffer) != VK_SUCCESS)
		throw std::runtime_error("failed to create buffer");

	// Get memory requirements including size, alignment and memory type 
	vkGetBufferMemoryRequirements(deviceManager.logicalDevice, uniformBufferVS.buffer, &memReqs);
	allocInfo.allocationSize = memReqs.size;

	// Get the memory type index that supports host visibile memory access
	// Most implementations offer multiple memory types and selecting the correct one to allocate memory from is crucial
	// We also want the buffer to be host coherent so we don't have to flush (or sync after every update.
	// Note: This may affect performance so you might not want to do this in a real world application that updates buffers on a regular base
	allocInfo.memoryTypeIndex = deviceManager.GetMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Allocate memory for the uniform buffer
	if (vkAllocateMemory(deviceManager.logicalDevice, &allocInfo, nullptr, &(uniformBufferVS.memory)) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate memory");

	// Bind memory to buffer
	if (vkBindBufferMemory(deviceManager.logicalDevice, uniformBufferVS.buffer, uniformBufferVS.memory, 0) != VK_SUCCESS)
		throw std::runtime_error("failed to bind buffer memory");

	// Store information in the uniform's descriptor that is used by the descriptor set
	uniformBufferVS.descriptor.buffer = uniformBufferVS.buffer;
	uniformBufferVS.descriptor.offset = 0;
	uniformBufferVS.descriptor.range = sizeof(uboVS);

	UpdateUniformBuffers();
}

void RendererApp::UpdateUniformBuffers()
{
	// Update matrices
	uboVS.projectionMatrix = glm::perspective(glm::radians(90.0f), (float)width / (float)height, 0.1f, 256.0f);

	uboVS.viewMatrix = glm::translate(glm::mat4(), position);

	uboVS.modelMatrix = glm::mat4();
	uboVS.modelMatrix = glm::translate(uboVS.modelMatrix, modelPosition);
	uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	uboVS.modelMatrix = glm::scale(uboVS.modelMatrix, glm::vec3(1.0f));

	// Map uniform buffer and update it
	uint8_t *pData;
	if (vkMapMemory(deviceManager.logicalDevice, uniformBufferVS.memory, 0, sizeof(uboVS), 0, (void **)&pData) != VK_SUCCESS)
		throw std::runtime_error("failed to map memory");

	memcpy(pData, &uboVS, sizeof(uboVS));

	// Unmap after data has been copied
	// Note: Since we requested a host coherent memory type for the uniform buffer, the write is instantly visible to the GPU
	vkUnmapMemory(deviceManager.logicalDevice, uniformBufferVS.memory);
}
