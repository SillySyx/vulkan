#include "simpleRenderer.h"

SimpleRenderer * SimpleRenderer::Create(SimpleRendererCreateInfo * pCreateInfo)
{
	auto renderer = new SimpleRenderer;
	renderer->pDevice = pCreateInfo->pDevice;
	renderer->pLogger = pCreateInfo->pLogger;
	renderer->pSurface = pCreateInfo->pSurface;

	renderer->Initialize();

	return renderer;
}

void SimpleRenderer::Initialize()
{
	GetGraphicsQueue();
	//GetSupportedDepthFormat();
	//CreateSynchronizationSemaphore(); image presentation
	//CreateSynchronizationSemaphore(); command submission
	//CreateSubmitInfoStructure();

	//CreateCommandPool();
	//CreateSwapChain();
	//CreateRenderCommandBuffers();
	//CreateDepthStencil();
	//CreateRenderPass();
	//CreatePipelineCache();
	//CreateFrameBuffers();
	//CreateFences();

	//CreateDescriptorSetLayout();
	//CreatePipelines();
	//CreateDescriptorPool();
	//CreateDescriptorSet();
	//CreateCommandBuffers();
}

void SimpleRenderer::GetGraphicsQueue()
{
	// Get a graphics queue from the device
	vkGetDeviceQueue(pDevice->logicalDevice, pDevice->graphicsQueueFamilyIndex, 0, &graphicsQueue);
	if (graphicsQueue == nullptr)
		throw "Failed to get graphics queue";

	pLogger->Info("SimpleRenderer graphics queue created");

	unsigned int queueCount;
	vkGetPhysicalDeviceQueueFamilyProperties(pDevice->physicalDevice, &queueCount, NULL);

	std::vector<VkQueueFamilyProperties> queueProps(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(pDevice->physicalDevice, &queueCount, queueProps.data());

	// Iterate over each queue to learn whether it supports presenting:
	// Find a queue with present support
	// Will be used to present the swap chain images to the windowing system
	std::vector<VkBool32> supportsPresent(queueCount);
	for (unsigned int i = 0; i < queueCount; i++)
		vkGetPhysicalDeviceSurfaceSupportKHR(pDevice->physicalDevice, i, pSurface->surface, &supportsPresent[i]);

	// Search for a graphics and a present queue in the array of queue
	// families, try to find one that supports both
	unsigned int graphicsQueueNodeIndex = UINT32_MAX;
	unsigned int presentQueueNodeIndex = UINT32_MAX;
	for (unsigned int i = 0; i < queueCount; i++)
	{
		if ((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
		{
			if (graphicsQueueNodeIndex == UINT32_MAX)
			{
				graphicsQueueNodeIndex = i;
			}

			if (supportsPresent[i] == VK_TRUE)
			{
				graphicsQueueNodeIndex = i;
				presentQueueNodeIndex = i;
				break;
			}
		}
	}
	if (presentQueueNodeIndex == UINT32_MAX)
	{
		// If there's no queue that supports both present and graphics
		// try to find a separate present queue
		for (unsigned int i = 0; i < queueCount; ++i)
		{
			if (supportsPresent[i] == VK_TRUE)
			{
				presentQueueNodeIndex = i;
				break;
			}
		}
	}

	// Exit if either a graphics or a presenting queue hasn't been found
	if (graphicsQueueNodeIndex == UINT32_MAX || presentQueueNodeIndex == UINT32_MAX)
		throw "Could not find a graphics and/or presenting queue!";

	if (graphicsQueueNodeIndex != presentQueueNodeIndex)
		throw "Separate graphics and presenting queues are not supported yet!";

	detectedGraphicsQueueIndex = graphicsQueueNodeIndex;
}
