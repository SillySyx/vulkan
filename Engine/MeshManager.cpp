#include "MeshManager.h"

void MeshManager::Initialize(DeviceManager* deviceManager, VkQueue* graphicsQueue)
{
	this->deviceManager = deviceManager;
	this->graphicsQueue = graphicsQueue;
}

MeshInstance* MeshManager::getInstance(std::string name)
{
	MeshInstance* instance = nullptr;
	for (auto i : instances)
	{
		if (i.name == name)
		{
			instance = &i;
			break;
		}
	}

	if (instance == nullptr) {
		instance = loadMesh(name);
	}

	return instance;
}

void MeshManager::buildBuffers(MeshInstance* instance, std::vector<Vertex> verticies, std::vector<uint32_t> indices)
{
	// A note on memory management in Vulkan in general:
	//	This is a very complex topic and while it's fine for an example application to to small individual memory allocations that is not
	//	what should be done a real-world application, where you should allocate large chunkgs of memory at once isntead.

	uint32_t vertexBufferSize = static_cast<uint32_t>(verticies.size()) * sizeof(Vertex);

	instance->indexCount = static_cast<uint32_t>(indices.size());
	uint32_t indexBufferSize = instance->indexCount * sizeof(uint32_t);

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
	if (vkCreateBuffer(deviceManager->logicalDevice, &vertexBufferInfo, nullptr, &stagingBuffers.vertices.buffer) != VK_SUCCESS)
		throw std::runtime_error("failed to create buffer");

	vkGetBufferMemoryRequirements(deviceManager->logicalDevice, stagingBuffers.vertices.buffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	// Request a host visible memory type that can be used to copy our data do
	// Also request it to be coherent, so that writes are visible to the GPU right after unmapping the buffer
	memAlloc.memoryTypeIndex = deviceManager->GetMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	if (vkAllocateMemory(deviceManager->logicalDevice, &memAlloc, nullptr, &stagingBuffers.vertices.memory) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate buffer");

	// Map and copy
	if (vkMapMemory(deviceManager->logicalDevice, stagingBuffers.vertices.memory, 0, memAlloc.allocationSize, 0, &data) != VK_SUCCESS)
		throw std::runtime_error("failed to map buffer");

	memcpy(data, verticies.data(), vertexBufferSize);
	vkUnmapMemory(deviceManager->logicalDevice, stagingBuffers.vertices.memory);
	if (vkBindBufferMemory(deviceManager->logicalDevice, stagingBuffers.vertices.buffer, stagingBuffers.vertices.memory, 0) != VK_SUCCESS)
		throw std::runtime_error("failed to bind buffer");

	// Create a device local buffer to which the (host local) vertex data will be copied and which will be used for rendering
	vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	if (vkCreateBuffer(deviceManager->logicalDevice, &vertexBufferInfo, nullptr, &instance->vertexBuffer) != VK_SUCCESS)
		throw std::runtime_error("failed to create buffer");

	vkGetBufferMemoryRequirements(deviceManager->logicalDevice, instance->vertexBuffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = deviceManager->GetMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if (vkAllocateMemory(deviceManager->logicalDevice, &memAlloc, nullptr, &instance->vertexBufferMemory) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate memory");
	if (vkBindBufferMemory(deviceManager->logicalDevice, instance->vertexBuffer, instance->vertexBufferMemory, 0) != VK_SUCCESS)
		throw std::runtime_error("failed to bind memory");

	// Index buffer
	VkBufferCreateInfo indexbufferInfo = {};
	indexbufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	indexbufferInfo.size = indexBufferSize;
	indexbufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	// Copy index data to a buffer visible to the host (staging buffer)
	if (vkCreateBuffer(deviceManager->logicalDevice, &indexbufferInfo, nullptr, &stagingBuffers.indices.buffer) != VK_SUCCESS)
		throw std::runtime_error("failed to create buffer");

	vkGetBufferMemoryRequirements(deviceManager->logicalDevice, stagingBuffers.indices.buffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = deviceManager->GetMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	if (vkAllocateMemory(deviceManager->logicalDevice, &memAlloc, nullptr, &stagingBuffers.indices.memory) != VK_SUCCESS)
		throw std::runtime_error("failed to create buffer");
	if (vkMapMemory(deviceManager->logicalDevice, stagingBuffers.indices.memory, 0, indexBufferSize, 0, &data) != VK_SUCCESS)
		throw std::runtime_error("failed to create buffer");
	memcpy(data, indices.data(), indexBufferSize);
	vkUnmapMemory(deviceManager->logicalDevice, stagingBuffers.indices.memory);

	if (vkBindBufferMemory(deviceManager->logicalDevice, stagingBuffers.indices.buffer, stagingBuffers.indices.memory, 0) != VK_SUCCESS)
		throw std::runtime_error("failed to create buffer");

	// Create destination buffer with device only visibility
	indexbufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	if (vkCreateBuffer(deviceManager->logicalDevice, &indexbufferInfo, nullptr, &instance->indexBuffer) != VK_SUCCESS)
		throw std::runtime_error("failed to create buffer");

	vkGetBufferMemoryRequirements(deviceManager->logicalDevice, instance->indexBuffer, &memReqs);

	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = deviceManager->GetMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if (vkAllocateMemory(deviceManager->logicalDevice, &memAlloc, nullptr, &instance->indexBufferMemory) != VK_SUCCESS)
		throw std::runtime_error("failed to create buffer");
	if (vkBindBufferMemory(deviceManager->logicalDevice, instance->indexBuffer, instance->indexBufferMemory, 0) != VK_SUCCESS)
		throw std::runtime_error("failed to create buffer");

	VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufferBeginInfo.pNext = nullptr;

	// Buffer copies have to be submitted to a queue, so we need a command buffer for them
	// Note: Some devices offer a dedicated transfer queue (with only the transfer bit set) that may be faster when doing lots of copies
	VkCommandBuffer copyCmd;

	VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
	cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufAllocateInfo.commandPool = deviceManager->commandPool;
	cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufAllocateInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(deviceManager->logicalDevice, &cmdBufAllocateInfo, &copyCmd) != VK_SUCCESS)
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
	vkCmdCopyBuffer(copyCmd, stagingBuffers.vertices.buffer, instance->vertexBuffer, 1, &copyRegion);

	// Index buffer
	copyRegion.size = indexBufferSize;
	vkCmdCopyBuffer(copyCmd, stagingBuffers.indices.buffer, instance->indexBuffer, 1, &copyRegion);

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
	if (vkCreateFence(deviceManager->logicalDevice, &fenceCreateInfo, nullptr, &fence) != VK_SUCCESS)
		throw std::runtime_error("failed to CreateFence");

	// Submit to the queue
	if (vkQueueSubmit(*graphicsQueue, 1, &submitInfo, fence) != VK_SUCCESS)
		throw std::runtime_error("failed to QueueSubmit");

	// Wait for the fence to signal that command buffer has finished executing
	if (vkWaitForFences(deviceManager->logicalDevice, 1, &fence, VK_TRUE, 100000000000) != VK_SUCCESS)
		throw std::runtime_error("failed to WaitForFences");

	vkDestroyFence(deviceManager->logicalDevice, fence, nullptr);
	vkFreeCommandBuffers(deviceManager->logicalDevice, deviceManager->commandPool, 1, &copyCmd);

	// Destroy staging buffers
	// Note: Staging buffer must not be deleted before the copies have been submitted and executed
	vkDestroyBuffer(deviceManager->logicalDevice, stagingBuffers.vertices.buffer, nullptr);
	vkFreeMemory(deviceManager->logicalDevice, stagingBuffers.vertices.memory, nullptr);
	vkDestroyBuffer(deviceManager->logicalDevice, stagingBuffers.indices.buffer, nullptr);
	vkFreeMemory(deviceManager->logicalDevice, stagingBuffers.indices.memory, nullptr);
}

MeshManager::~MeshManager()
{
	if (deviceManager != nullptr)
	{
		for (auto instance : instances)
		{
			if (instance.vertexBuffer != VK_NULL_HANDLE)
				vkDestroyBuffer(deviceManager->logicalDevice, instance.vertexBuffer, nullptr);

			if (instance.vertexBufferMemory != VK_NULL_HANDLE)
				vkFreeMemory(deviceManager->logicalDevice, instance.vertexBufferMemory, nullptr);

			if (instance.indexBuffer != VK_NULL_HANDLE)
				vkDestroyBuffer(deviceManager->logicalDevice, instance.indexBuffer, nullptr);

			if (instance.indexBufferMemory != VK_NULL_HANDLE)
				vkFreeMemory(deviceManager->logicalDevice, instance.indexBufferMemory, nullptr);
		}
	}
}

MeshInstance* MeshManager::loadMesh(std::string name)
{
	if (name == "defaultBox")
	{
		std::vector<Vertex> vertices =
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
		std::vector<uint32_t> indices = {
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
		MeshInstance instance;
		instance.name = "defaultBox";
		instance.position = glm::vec3(0.f);
		instance.rotation = glm::vec3(0.f);
		instance.scale = glm::vec3(1.f);

		buildBuffers(&instance, vertices, indices);

		return &instance;
	}
	
	return nullptr;
}
