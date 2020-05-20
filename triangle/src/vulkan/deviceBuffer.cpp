#include "deviceBuffer.h"

DeviceBuffer::~DeviceBuffer()
{
	vkDestroyBuffer(pDevice->logicalDevice, buffer, nullptr);
	buffer = nullptr;

	vkFreeMemory(pDevice->logicalDevice, memory, nullptr);
	memory = nullptr;
}

void DeviceBuffer::StageBuffer(void * source, VkDeviceSize size, VkBufferUsageFlags flags, VkCommandPool commandPool, VkQueue queue)
{
	pLogger->Info("DeviceBuffer::StageBuffer started");

	// Static data like vertex and index buffer should be stored on the device memory 
	// for optimal (and fastest) access by the GPU

	// To achieve this we use so-called "staging buffers" :
	// - Create a buffer that's visible to the host (and can be mapped)
	// - Copy the data to this buffer
	// - Create another buffer that's local on the device (VRAM) with the same size
	// - Copy the data from the host to the device using a command buffer
	// - Delete the host visible (staging) buffer
	// - Use the device local buffers for rendering

	VkDeviceMemory stagingMemory;
	VkBuffer stagingBuffer;
	void * data;

	VkBufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size = size;
	createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	// Create a host buffer 
	if (vkCreateBuffer(pDevice->logicalDevice, &createInfo, nullptr, &stagingBuffer) != VK_SUCCESS)
		throw "Failed to create host buffer";

	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(pDevice->logicalDevice, stagingBuffer, &memReqs);

	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = pDevice->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (vkAllocateMemory(pDevice->logicalDevice, &memAlloc, nullptr, &stagingMemory) != VK_SUCCESS)
		throw "Failed to allocate host buffer memory";

	// Map and copy
	if (vkMapMemory(pDevice->logicalDevice, stagingMemory, 0, memAlloc.allocationSize, 0, &data) != VK_SUCCESS)
		throw "Failed to map memory";

	memcpy(data, source, size);
	vkUnmapMemory(pDevice->logicalDevice, stagingMemory);

	if (vkBindBufferMemory(pDevice->logicalDevice, stagingBuffer, stagingMemory, 0) != VK_SUCCESS)
		throw "Failed to bind host buffer memory";

	// Create a device local buffer to which the (host local) vertex data will be copied and which will be used for rendering
	createInfo.usage = flags;
	if (vkCreateBuffer(pDevice->logicalDevice, &createInfo, nullptr, &buffer) != VK_SUCCESS)
		throw "Failed to create device buffer";

	vkGetBufferMemoryRequirements(pDevice->logicalDevice, buffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = pDevice->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if (vkAllocateMemory(pDevice->logicalDevice, &memAlloc, nullptr, &memory) != VK_SUCCESS)
		throw "Failed to allocate device buffer memory";

	if (vkBindBufferMemory(pDevice->logicalDevice, buffer, memory, 0) != VK_SUCCESS)
		throw "Failed to bind device buffer memory";

	// Buffer copies have to be submitted to a queue, so we need a command buffer for them
	// Note: Some devices offer a dedicated transfer queue (with only the transfer bit set) that may be faster when doing lots of copies

	VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
	cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufAllocateInfo.commandPool = commandPool;
	cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufAllocateInfo.commandBufferCount = 1;

	VkCommandBuffer copyCmd;
	if (vkAllocateCommandBuffers(pDevice->logicalDevice, &cmdBufAllocateInfo, &copyCmd) != VK_SUCCESS)
		throw "Failed to allocate command buffers";

	VkCommandBufferBeginInfo cmdBufInfo = {};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufInfo.pNext = NULL;
	if (vkBeginCommandBuffer(copyCmd, &cmdBufInfo) != VK_SUCCESS)
		throw "Failed to begin command buffer";

	// Put buffer region copies into command buffer
	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	vkCmdCopyBuffer(copyCmd, stagingBuffer, buffer, 1, &copyRegion);

	// Flushing the command buffer will also submit it to the queue and uses a fence to ensure that all commands have been executed before returning
	if (vkEndCommandBuffer(copyCmd) != VK_SUCCESS)
		throw "Failed to end command buffer";

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &copyCmd;

	// Create fence to ensure that the command buffer has finished executing
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = 0;

	VkFence fence;
	if (vkCreateFence(pDevice->logicalDevice, &fenceCreateInfo, nullptr, &fence) != VK_SUCCESS)
		throw "failed to create fence";

	// Submit to the queue
	if (vkQueueSubmit(queue, 1, &submitInfo, fence) != VK_SUCCESS)
		throw "Failed to submit queue";

	// Wait for the fence to signal that command buffer has finished executing
	if (vkWaitForFences(pDevice->logicalDevice, 1, &fence, VK_TRUE, 100000000000) != VK_SUCCESS)
		throw "failed to wait for fences";

	vkDestroyFence(pDevice->logicalDevice, fence, nullptr);
	vkFreeCommandBuffers(pDevice->logicalDevice, commandPool, 1, &copyCmd);

	// Destroy staging buffers
	// Note: Staging buffer must not be deleted before the copies have been submitted and executed
	vkDestroyBuffer(pDevice->logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(pDevice->logicalDevice, stagingMemory, nullptr);

	pLogger->Info("DeviceBuffer::StageBuffer done");
}
