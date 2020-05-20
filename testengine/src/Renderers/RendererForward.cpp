#include "RendererForward.h"

void RendererForward::Initiate()
{
	CreateCommandPool();
	CreateSwapChain();
	CreateRenderCommandBuffers();
	CreateDepthStencil();
	CreateRenderPass();
	CreatePipelineCache();
	CreateFrameBuffers();
	CreateFences();
}

RendererForward::~RendererForward()
{
	DestroyFences();
	DestroyFrameBuffers();
	DestoryPipelineCache();
	DestoryRenderPass();
	DestoryDepthStencil();
	DestoryRenderCommandBuffers();
	DestroySwapChain();
	DestroyCommandPool();
}

inline void RendererForward::Render(std::function<void()> fnc)
{
	for (uint32_t bufferIndex = 0; bufferIndex < swapChainImageCount; bufferIndex++)
	{
		// Get next image in the swap chain (back/front buffer)
		if (vkAcquireNextImageKHR(logicalDevice, swapChain, UINT64_MAX, semaphorePresentation, (VkFence)nullptr, &bufferIndex) != VK_SUCCESS)
			throw "Failed to accuire next image";

		if (vkWaitForFences(logicalDevice, 1, &waitFences[bufferIndex], VK_TRUE, UINT64_MAX) != VK_SUCCESS)
			throw "Failed to wait for fences";

		// Use a fence to wait until the command buffer has finished execution before using it again
		if (vkResetFences(logicalDevice, 1, &waitFences[bufferIndex]) != VK_SUCCESS)
			throw "Failed to reset fences";

		// Pipeline stage at which the queue submission will wait (via pWaitSemaphores)
		VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		// The submit info structure specifices a command buffer queue submission batch
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pWaitDstStageMask = &waitStageMask;									// Pointer to the list of pipeline stages that the semaphore waits will occur at
		submitInfo.pWaitSemaphores = &semaphorePresentation;					// Semaphore(s) to wait upon before the submitted command buffer starts executing
		submitInfo.waitSemaphoreCount = 1;												// One wait semaphore																				
		submitInfo.pSignalSemaphores = &semaphoreRender;						// Semaphore(s) to be signaled when command buffers have completed
		submitInfo.signalSemaphoreCount = 1;											// One signal semaphore
		submitInfo.pCommandBuffers = &renderCommandBuffers[bufferIndex];    // Command buffers(s) to execute in this batch (submission)
		submitInfo.commandBufferCount = 1;												// One command buffer

																						// Submit to the graphics queue passing a wait fence
		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, waitFences[bufferIndex]) != VK_SUCCESS)
			throw "Failed to submit graphics queue";

		if (fnc != 0)
			fnc();

		// Present the current buffer to the swap chain
		// Pass the semaphore signaled by the command buffer submission from the submit info as the wait semaphore for swap chain presentation
		// This ensures that the image is not presented to the windowing system until all commands have been submitted
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = NULL;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapChain;
		presentInfo.pImageIndices = &bufferIndex;
		presentInfo.pWaitSemaphores = &semaphoreRender;
		presentInfo.waitSemaphoreCount = 1;

		if (vkQueuePresentKHR(graphicsQueue, &presentInfo) != VK_SUCCESS)
			throw "Failed to present queue";
	}
}

void RendererForward::ResizeResources(uint32_t width, uint32_t height)
{
	logger.Info("RendererForward::ResizeResources " + std::to_string(width) + "x" + std::to_string(height));

	this->width = width;
	this->height = height;

	vkDeviceWaitIdle(logicalDevice);

	DestroyFrameBuffers();
	DestoryDepthStencil();
	DestoryRenderCommandBuffers();
	DestroySwapChain();

	CreateSwapChain();
	CreateRenderCommandBuffers();
	CreateDepthStencil();
	CreateFrameBuffers();

	logger.Info("RendererForward::ResizeResources done");
}

void RendererForward::CreateSwapChain()
{
	logger.Info("RendererForward::CreateSwapChain ...");

	VkSwapchainKHR oldSwapchain = swapChain;

	VkSurfaceCapabilitiesKHR surfCaps;
	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfCaps) != VK_SUCCESS)
		throw "failed to get device surface capabilities";

	uint32_t presentModeCount;
	if (vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr) != VK_SUCCESS)
		throw "failed to get device present modes count";

	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	if (vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data()) != VK_SUCCESS)
		throw "failed to get device present modes";

	VkExtent2D swapchainExtent = {};
	// If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
	if (surfCaps.currentExtent.width == (uint32_t)-1)
	{
		// If the surface size is undefined, the size is set to
		// the size of the images requested.
		swapchainExtent.width = width;
		swapchainExtent.height = height;
	}
	else
	{
		// If the surface size is defined, the swap chain size must match
		swapchainExtent = surfCaps.currentExtent;
		width = surfCaps.currentExtent.width;
		height = surfCaps.currentExtent.height;
	}

	// The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
	// This mode waits for the vertical blank ("v-sync")
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

	// If v-sync is not requested, try to find a mailbox mode
	// It's the lowest latency non-tearing present mode available
	if (!vsync)
	{
		for (size_t i = 0; i < presentModeCount; i++)
		{
			if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			}
			if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
			{
				swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}
	}

	// Determine the number of images
	uint32_t desiredNumberOfSwapchainImages = surfCaps.minImageCount + 1;
	if ((surfCaps.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfCaps.maxImageCount))
	{
		desiredNumberOfSwapchainImages = surfCaps.maxImageCount;
	}

	// Find the transformation of the surface
	VkSurfaceTransformFlagsKHR preTransform;
	if (surfCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		// We prefer a non-rotated transform
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		preTransform = surfCaps.currentTransform;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.pNext = NULL;
	createInfo.surface = surface;
	createInfo.minImageCount = desiredNumberOfSwapchainImages;
	createInfo.imageFormat = surfaceColorFormat;
	createInfo.imageColorSpace = surfaceColorSpace;
	createInfo.imageExtent = { swapchainExtent.width, swapchainExtent.height };
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
	createInfo.imageArrayLayers = 1;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = 0;
	createInfo.pQueueFamilyIndices = NULL;
	createInfo.presentMode = swapchainPresentMode;
	createInfo.oldSwapchain = oldSwapchain;
	// Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
	createInfo.clipped = VK_TRUE;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	// Set additional usage flag for blitting from the swapchain images if supported
	VkFormatProperties formatProps;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, surfaceColorFormat, &formatProps);
	if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT) {
		createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}

	if (vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
		throw "failed to create swap chain";

	logger.Info("RendererForward::CreateSwapChain " + logger.GetAddr(swapChain));

	// If an existing swap chain is re-created, destroy the old swap chain
	// This also cleans up all the presentable images
	if (oldSwapchain != VK_NULL_HANDLE)
	{
		for (uint32_t i = 0; i < swapChainImageCount; i++)
		{
			vkDestroyImageView(logicalDevice, swapChainImageViews[i], nullptr);
		}
		vkDestroySwapchainKHR(logicalDevice, oldSwapchain, nullptr);
	}

	if (vkGetSwapchainImagesKHR(logicalDevice, swapChain, &swapChainImageCount, nullptr))
		throw "Failed to get swap chain images count";

	swapChainImages.resize(swapChainImageCount);
	swapChainImageViews.resize(swapChainImageCount);

	// Get the swap chain images
	if (vkGetSwapchainImagesKHR(logicalDevice, swapChain, &swapChainImageCount, swapChainImages.data()))
		throw "Failed to get swap chain images";

	for (uint32_t i = 0; i < swapChainImageCount; i++)
	{
		VkImageViewCreateInfo colorAttachmentView = {};
		colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		colorAttachmentView.pNext = NULL;
		colorAttachmentView.format = surfaceColorFormat;
		colorAttachmentView.components = {
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A
		};
		colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		colorAttachmentView.subresourceRange.baseMipLevel = 0;
		colorAttachmentView.subresourceRange.levelCount = 1;
		colorAttachmentView.subresourceRange.baseArrayLayer = 0;
		colorAttachmentView.subresourceRange.layerCount = 1;
		colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		colorAttachmentView.flags = 0;
		colorAttachmentView.image = swapChainImages[i];

		if (vkCreateImageView(logicalDevice, &colorAttachmentView, nullptr, &swapChainImageViews[i]))
			throw "failed to create image view";

		logger.Info("RendererForward::CreateSwapChain CreateImageView " + logger.GetAddr(swapChainImageViews[i]));
	}
}

void RendererForward::DestroySwapChain()
{
	if (swapChain)
	{
		logger.Info("RendererForward::DestroySwapChain " + logger.GetAddr(swapChain));

		for (uint32_t i = 0; i < swapChainImageCount; i++)
		{
			logger.Info("RendererForward::DestroySwapChain DestroyImageView " + logger.GetAddr(swapChainImageViews[i]));

			vkDestroyImageView(logicalDevice, swapChainImageViews[i], nullptr);

			logger.Info("RendererForward::DestroySwapChain DestroyImageView done");
		}

		vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
		swapChain = nullptr;

		logger.Info("RendererForward::DestroySwapChain done");
	}
}

void RendererForward::CreateRenderCommandBuffers()
{
	logger.Info("RendererForward::CreateRenderCommandBuffers ...");

	renderCommandBuffers.resize(swapChainImageCount);

	VkCommandBufferAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool = commandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = static_cast<uint32_t>(renderCommandBuffers.size());

	if (vkAllocateCommandBuffers(logicalDevice, &allocateInfo, renderCommandBuffers.data()))
		throw "Failed to allocate rendering command buffers";

	logger.Info("RendererForward::CreateRenderCommandBuffers " + std::to_string(renderCommandBuffers.size()) + " buffer(s) done");
}

void RendererForward::DestoryRenderCommandBuffers()
{
	if (logicalDevice && commandPool)
	{
		logger.Info("RendererForward::DestoryRenderCommandBuffers " + std::to_string(renderCommandBuffers.size()) + " buffer(s)");

		vkFreeCommandBuffers(logicalDevice, commandPool, static_cast<uint32_t>(renderCommandBuffers.size()), renderCommandBuffers.data());

		logger.Info("RendererForward::DestoryRenderCommandBuffers done");
	}
}

void RendererForward::CreateDepthStencil()
{
	logger.Info("RendererForward::CreateDepthStencil ...");

	VkImageCreateInfo image = {};
	image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image.pNext = NULL;
	image.imageType = VK_IMAGE_TYPE_2D;
	image.format = depthFormat;
	image.extent = { width, height, 1 };
	image.mipLevels = 1;
	image.arrayLayers = 1;
	image.samples = VK_SAMPLE_COUNT_1_BIT;
	image.tiling = VK_IMAGE_TILING_OPTIMAL;
	image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	image.flags = 0;

	if (vkCreateImage(logicalDevice, &image, nullptr, &depthStencilImage) != VK_SUCCESS)
		throw "failed to create depth stencil image";

	logger.Info("RendererForward::CreateDepthStencil DepthStencilImage " + logger.GetAddr(depthStencilImage));

	VkMemoryAllocateInfo mem_alloc = {};
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc.pNext = NULL;
	mem_alloc.allocationSize = 0;
	mem_alloc.memoryTypeIndex = 0;

	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(logicalDevice, depthStencilImage, &memReqs);
	mem_alloc.allocationSize = memReqs.size;
	mem_alloc.memoryTypeIndex = GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(logicalDevice, &mem_alloc, nullptr, &depthStencilMemory) != VK_SUCCESS)
		throw "Failed to allocate depth stencil memory";

	logger.Info("RendererForward::CreateDepthStencil DepthStencilMemory " + logger.GetAddr(depthStencilMemory));

	if (vkBindImageMemory(logicalDevice, depthStencilImage, depthStencilMemory, 0) != VK_SUCCESS)
		throw "Failed to bind depth stencil image memory";

	logger.Info("RendererForward::CreateDepthStencil BindImageMemory done");

	VkImageViewCreateInfo depthStencilView = {};
	depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depthStencilView.pNext = NULL;
	depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthStencilView.format = depthFormat;
	depthStencilView.flags = 0;
	depthStencilView.subresourceRange = {};
	depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	depthStencilView.subresourceRange.baseMipLevel = 0;
	depthStencilView.subresourceRange.levelCount = 1;
	depthStencilView.subresourceRange.baseArrayLayer = 0;
	depthStencilView.subresourceRange.layerCount = 1;
	depthStencilView.image = depthStencilImage;

	if (vkCreateImageView(logicalDevice, &depthStencilView, nullptr, &depthStencilImageView))
		throw "Failed to create depth stencil image view";

	logger.Info("RendererForward::CreateDepthStencil DepthStencilImageView " + logger.GetAddr(depthStencilImageView));
}

void RendererForward::DestoryDepthStencil()
{
	if (depthStencilImageView)
	{
		logger.Info("RendererForward::DestoryDepthStencil DepthStencilImageView " + logger.GetAddr(depthStencilImageView));

		vkDestroyImageView(logicalDevice, depthStencilImageView, nullptr);
		depthStencilImageView = nullptr;

		logger.Info("RendererForward::DestoryDepthStencil DepthStencilImageView done");
	}

	if (depthStencilImage)
	{
		logger.Info("RendererForward::DestoryDepthStencil DepthStencilImage " + logger.GetAddr(depthStencilImage));

		vkDestroyImage(logicalDevice, depthStencilImage, nullptr);
		depthStencilImage = nullptr;

		logger.Info("RendererForward::DestoryDepthStencil DepthStencilImage done");
	}

	if (depthStencilMemory)
	{
		logger.Info("RendererForward::DestoryDepthStencil DepthStencilMemory " + logger.GetAddr(depthStencilMemory));

		vkFreeMemory(logicalDevice, depthStencilMemory, nullptr);

		logger.Info("RendererForward::DestoryDepthStencil DepthStencilMemory done");
	}
}

void RendererForward::CreateRenderPass()
{
	logger.Info("RendererForward::CreateRenderPass ...");

	std::array<VkAttachmentDescription, 2> attachments = {};
	// Color attachment
	attachments[0].format = surfaceColorFormat;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	// Depth attachment
	attachments[1].format = depthFormat;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorReference = {};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorReference;
	subpassDescription.pDepthStencilAttachment = &depthReference;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = nullptr;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = nullptr;
	subpassDescription.pResolveAttachments = nullptr;

	// Subpass dependencies for layout transitions
	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	if (vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass))
		throw "Failed to create render pass";

	logger.Info("RendererForward::CreateRenderPass " + logger.GetAddr(renderPass));
}

void RendererForward::DestoryRenderPass()
{
	if (renderPass)
	{
		logger.Info("RendererForward::DestoryRenderPass " + logger.GetAddr(renderPass));

		vkDestroyRenderPass(logicalDevice, renderPass, nullptr);
		renderPass = nullptr;

		logger.Info("RendererForward::DestoryRenderPass done");
	}
}

void RendererForward::CreatePipelineCache()
{
	logger.Info("RendererForward::CreatePipelineCache ...");

	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

	if (vkCreatePipelineCache(logicalDevice, &pipelineCacheCreateInfo, nullptr, &pipelineCache))
		throw "Failed to create pipeline cache";

	logger.Info("RendererForward::CreatePipelineCache " + logger.GetAddr(pipelineCache));
}

void RendererForward::DestoryPipelineCache()
{
	if (pipelineCache)
	{
		logger.Info("RendererForward::DestoryPipelineCache " + logger.GetAddr(pipelineCache));

		vkDestroyPipelineCache(logicalDevice, pipelineCache, nullptr);
		pipelineCache = nullptr;

		logger.Info("RendererForward::DestoryPipelineCache done");
	}
}

void RendererForward::CreateFrameBuffers()
{
	logger.Info("RendererForward::CreateFrameBuffers ...");

	VkImageView attachments[2];

	// Depth/Stencil attachment is the same for all frame buffers
	attachments[1] = depthStencilImageView;

	VkFramebufferCreateInfo frameBufferCreateInfo = {};
	frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferCreateInfo.pNext = NULL;
	frameBufferCreateInfo.renderPass = renderPass;
	frameBufferCreateInfo.attachmentCount = 2;
	frameBufferCreateInfo.pAttachments = attachments;
	frameBufferCreateInfo.width = width;
	frameBufferCreateInfo.height = height;
	frameBufferCreateInfo.layers = 1;

	// Create frame buffers for every swap chain image
	frameBuffers.resize(swapChainImageCount);
	for (uint32_t i = 0; i < frameBuffers.size(); i++)
	{
		attachments[0] = swapChainImageViews[i];
		if (vkCreateFramebuffer(logicalDevice, &frameBufferCreateInfo, nullptr, &frameBuffers[i]))
			throw "Failed to create frame buffers";

		logger.Info("RendererForward::CreateFrameBuffers " + logger.GetAddr(frameBuffers[i]));
	}
}

void RendererForward::DestroyFrameBuffers()
{
	logger.Info("RendererForward::DestroyFrameBuffers ...");

	for (uint32_t i = 0; i < frameBuffers.size(); i++)
	{
		logger.Info("RendererForward::DestroyFrameBuffers " + logger.GetAddr(frameBuffers[i]));

		vkDestroyFramebuffer(logicalDevice, frameBuffers[i], nullptr);
		frameBuffers[i] = nullptr;
	}

	logger.Info("RendererForward::DestroyFrameBuffers done");
}

void RendererForward::CreateFences()
{
	logger.Info("RendererForward::CreateFences ...");

	// Fences (Used to check draw command buffer completion)
	// Create in signaled state so we don't wait on first render of each command buffer

	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	waitFences.resize(renderCommandBuffers.size());
	for (auto& fence : waitFences)
	{
		if (vkCreateFence(logicalDevice, &fenceCreateInfo, nullptr, &fence) != VK_SUCCESS)
			throw "Failed to create fence";

		logger.Info("RendererForward::CreateFences " + logger.GetAddr(fence));
	}
}

void RendererForward::DestroyFences()
{
	logger.Info("RendererForward::DestroyFences ...");

	for (auto fence : waitFences)
	{
		logger.Info("RendererForward::DestroyFences " + logger.GetAddr(fence));

		vkDestroyFence(logicalDevice, fence, nullptr);
		fence = nullptr;
	}

	logger.Info("RendererForward::DestroyFences done");
}

void RendererForward::CreateCommandPool()
{
	logger.Info("RendererForward::CreateCommandPool ...");

	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(logicalDevice, &cmdPoolInfo, nullptr, &commandPool) != VK_SUCCESS)
		throw "Failed to create command pool";

	logger.Info("RendererForward::CreateCommandPool " + logger.GetAddr(commandPool));
}

void RendererForward::DestroyCommandPool()
{
	if (commandPool)
	{
		logger.Info("RendererForward::DestroyCommandPool " + logger.GetAddr(commandPool));

		vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
		commandPool = nullptr;

		logger.Info("RendererForward::DestroyCommandPool done");
	}
}
