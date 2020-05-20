#include "Renderer.h"

void Renderer::Initialize(DeviceManager* deviceManager, VkSurfaceKHR surface, int width, int height)
{
	this->width = width;
	this->height = height;

	this->deviceManager = deviceManager;
	this->surface = surface;

	CreateGraphicsQueue();
	CreateSwapChain();
	CreateDrawCommandBuffers();
	CreateDepthStencil();
	CreateRenderPass();
	CreatePipelineCache();
	CreateDrawFrameBuffers();
	CreateSemaphores();
	CreateDescriptorSetLayout();
	CreatePipelineLayout();
	CreatePipelineVertexInputState();
	CreateDescriptorPool();
	CreateDescriptorSet();
}

void Renderer::BuildPipelines(std::vector<VkPipelineShaderStageCreateInfo> shaderStages)
{
	// Create the graphics pipeline used in this example
	// Vulkan uses the concept of rendering pipelines to encapsulate fixed states, replacing OpenGL's complex state machine
	// A pipeline is then stored and hashed on the GPU making pipeline changes very fast
	// Note: There are still a few dynamic states that are not directly part of the pipeline (but the info that they are used is)

	// Construct the differnent states making up the pipeline

	// Input assembly state describes how primitives are assembled
	// This pipeline will assemble vertex data as a triangle lists (though we only use one triangle)
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
	inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	// Rasterization state
	VkPipelineRasterizationStateCreateInfo rasterizationState = {};
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	rasterizationState.depthBiasEnable = VK_FALSE;
	rasterizationState.lineWidth = 1.0f;

	// Color blend state describes how blend factors are calculated (if used)
	// We need one blend attachment state per color attachment (even if blending is not used
	VkPipelineColorBlendAttachmentState blendAttachmentState[1] = {};
	blendAttachmentState[0].colorWriteMask = 0xf;
	blendAttachmentState[0].blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlendState = {};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = blendAttachmentState;

	// Viewport state sets the number of viewports and scissor used in this pipeline
	// Note: This is actually overriden by the dynamic states (see below)
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	// Enable dynamic states
	// Most states are baked into the pipeline, but there are still a few dynamic states that can be changed within a command buffer
	// To be able to change these we need do specify which dynamic states will be changed using this pipeline. Their actual states are set later on in the command buffer.
	// For this example we will set the viewport and scissor using dynamic states
	std::vector<VkDynamicState> dynamicStateEnables;
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);

	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates = dynamicStateEnables.data();
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

	// Depth and stencil state containing depth and stencil compare and test operations
	// We only use depth tests and want depth tests and writes to be enabled and compare with less or equal
	VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.depthTestEnable = VK_TRUE;
	depthStencilState.depthWriteEnable = VK_TRUE;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilState.depthBoundsTestEnable = VK_FALSE;
	depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
	depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
	depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
	depthStencilState.stencilTestEnable = VK_FALSE;
	depthStencilState.front = depthStencilState.back;

	// Multi sampling state
	// This example does not make use fo multi sampling (for anti-aliasing), the state must still be set and passed to the pipeline
	VkPipelineMultisampleStateCreateInfo multisampleState = {};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleState.pSampleMask = nullptr;

	// Set pipeline shader stage info
	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	// The layout used for this pipeline (can be shared among multiple pipelines using the same layout)
	pipelineCreateInfo.layout = resources.pipelineLayout;
	// Renderpass this pipeline is attached to
	pipelineCreateInfo.renderPass = resources.renderPass;
	pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCreateInfo.pStages = shaderStages.data();

	// Assign the pipeline states to the pipeline creation info structure
	pipelineCreateInfo.pVertexInputState = &resources.pipelineVertexInputState;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.renderPass = resources.renderPass;
	pipelineCreateInfo.pDynamicState = &dynamicState;

	// Create rendering pipeline using the specified states
	if (vkCreateGraphicsPipelines(deviceManager->logicalDevice, resources.pipelineCache, 1, &pipelineCreateInfo, nullptr, &resources.pipeline) != VK_SUCCESS)
		throw std::runtime_error("failed to create graphics pipeline");
}

void Renderer::BuildDescriptorSets(VkDescriptorBufferInfo* descriptorBufferInfo)
{
	// Update the descriptor set determining the shader binding points
	// For every binding point used in a shader there needs to be one
	// descriptor set matching that binding point

	VkWriteDescriptorSet writeDescriptorSet = {};

	// Binding 0 : Uniform buffer
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.dstSet = resources.descriptorSet;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSet.pBufferInfo = descriptorBufferInfo;

	// Binds this uniform buffer to binding point 0
	writeDescriptorSet.dstBinding = 0;

	vkUpdateDescriptorSets(deviceManager->logicalDevice, 1, &writeDescriptorSet, 0, nullptr);
}

void Renderer::BuildDrawCommandBuffers(VkBuffer* vertices, VkBuffer* indices, uint32_t indexCount)
{
	VkCommandBufferBeginInfo cmdBufInfo = {};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufInfo.pNext = nullptr;

	// Set clear values for all framebuffer attachments with loadOp set to clear
	// We use two attachments (color and depth) that are cleared at the start of the subpass and as such we need to set clear values for both
	VkClearValue clearValues[2];
	clearValues[0].color = clearColor;
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = nullptr;
	renderPassBeginInfo.renderPass = resources.renderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = width;
	renderPassBeginInfo.renderArea.extent.height = height;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;

	for (int32_t i = 0; i < resources.drawCommandBuffers.size(); ++i)
	{
		// Set target frame buffer
		renderPassBeginInfo.framebuffer = resources.frameBuffers[i];

		if (vkBeginCommandBuffer(resources.drawCommandBuffers[i], &cmdBufInfo) != VK_SUCCESS)
			throw std::runtime_error("failed to begin CommandBuffer");

		// Start the first sub pass specified in our default render pass setup by the base class
		// This will clear the color and depth attachment
		vkCmdBeginRenderPass(resources.drawCommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Update dynamic viewport state
		VkViewport viewport = {};
		viewport.height = (float)height;
		viewport.width = (float)width;
		viewport.minDepth = (float) 0.0f;
		viewport.maxDepth = (float) 1.0f;
		vkCmdSetViewport(resources.drawCommandBuffers[i], 0, 1, &viewport);

		// Update dynamic scissor state
		VkRect2D scissor = {};
		scissor.extent.width = width;
		scissor.extent.height = height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(resources.drawCommandBuffers[i], 0, 1, &scissor);

		// Bind descriptor sets describing shader binding points
		vkCmdBindDescriptorSets(resources.drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, resources.pipelineLayout, 0, 1, &resources.descriptorSet, 0, nullptr);

		// Bind the rendering pipeline
		// The pipeline (state object) contains all states of the rendering pipeline, binding it will set all the states specified at pipeline creation time
		vkCmdBindPipeline(resources.drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, resources.pipeline);

		// Bind triangle vertex buffer (contains position and colors)
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(resources.drawCommandBuffers[i], 0, 1, vertices, offsets);

		// Bind triangle index buffer
		vkCmdBindIndexBuffer(resources.drawCommandBuffers[i], *indices, 0, VK_INDEX_TYPE_UINT32);

		// Draw indexed triangle
		vkCmdDrawIndexed(resources.drawCommandBuffers[i], indexCount, 1, 0, 0, 1);

		vkCmdEndRenderPass(resources.drawCommandBuffers[i]);

		// Ending the render pass will add an implicit barrier transitioning the frame buffer color attachment to 
		// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR for presenting it to the windowing system

		if (vkEndCommandBuffer(resources.drawCommandBuffers[i]) != VK_SUCCESS)
			throw std::runtime_error("failed to end CommandBuffer");
	}
}

void Renderer::RenderFrame()
{
	// Get next image in the swap chain (back/front buffer)
	if (vkAcquireNextImageKHR(deviceManager->logicalDevice, resources.swapchain, UINT64_MAX, resources.semaphorePresent, (VkFence)nullptr, &resources.swapchainCurrentImage) != VK_SUCCESS)
		throw std::runtime_error("failed to AcquireNextImage");

	// Use a fence to wait until the command buffer has finished execution before using it again
	if (vkWaitForFences(deviceManager->logicalDevice, 1, &resources.drawFences[resources.swapchainCurrentImage], VK_TRUE, UINT64_MAX) != VK_SUCCESS)
		throw std::runtime_error("failed to Wait for Fences");


	if (vkResetFences(deviceManager->logicalDevice, 1, &resources.drawFences[resources.swapchainCurrentImage]) != VK_SUCCESS)
		throw std::runtime_error("failed to reset Fences");

	// Pipeline stage at which the queue submission will wait (via pWaitSemaphores)
	VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	// The submit info structure specifices a command buffer queue submission batch
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pWaitDstStageMask = &waitStageMask;									// Pointer to the list of pipeline stages that the semaphore waits will occur at
	submitInfo.pWaitSemaphores = &resources.semaphorePresent;							// Semaphore(s) to wait upon before the submitted command buffer starts executing
	submitInfo.waitSemaphoreCount = 1;												// One wait semaphore																				
	submitInfo.pSignalSemaphores = &resources.semaphoreRender;						// Semaphore(s) to be signaled when command buffers have completed
	submitInfo.signalSemaphoreCount = 1;											// One signal semaphore
	submitInfo.pCommandBuffers = &resources.drawCommandBuffers[resources.swapchainCurrentImage];	// Command buffers(s) to execute in this batch (submission)
	submitInfo.commandBufferCount = 1;												// One command buffer

																					// Submit to the graphics queue passing a wait fence
	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, resources.drawFences[resources.swapchainCurrentImage]) != VK_SUCCESS)
		throw std::runtime_error("failed to QueueSubmit");

	// Present the current buffer to the swap chain
	// Pass the semaphore signaled by the command buffer submission from the submit info as the wait semaphore for swap chain presentation
	// This ensures that the image is not presented to the windowing system until all commands have been submitted
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = NULL;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &resources.swapchain;
	presentInfo.pImageIndices = &resources.swapchainCurrentImage;

	// Check if a wait semaphore has been specified to wait for before presenting the image
	if (resources.semaphoreRender != VK_NULL_HANDLE)
	{
		presentInfo.pWaitSemaphores = &resources.semaphoreRender;
		presentInfo.waitSemaphoreCount = 1;
	}

	if (vkQueuePresentKHR(graphicsQueue, &presentInfo) != VK_SUCCESS)
		throw std::runtime_error("failed to QueuePresent");
}

void Renderer::ResizeResources(std::vector<VkPipelineShaderStageCreateInfo> shaderStages, VkDescriptorBufferInfo* descriptorBufferInfo, VkBuffer* vertices, VkBuffer* indices, uint32_t indexCount)
{
	CreateSwapChain();
	CreateDrawCommandBuffers();
	CreateDepthStencil();
	CreateDrawFrameBuffers();
	CreateDescriptorPool();
	CreateDescriptorSet();

	BuildDescriptorSets(descriptorBufferInfo);
	BuildDrawCommandBuffers(vertices, indices, indexCount);
}

Renderer::~Renderer()
{
	vkDestroyPipeline(deviceManager->logicalDevice, resources.pipeline, nullptr);
	resources.pipeline = NULL;

	vkDestroyDescriptorPool(deviceManager->logicalDevice, resources.descriptorPool, nullptr);
	resources.descriptorPool = NULL;

	vkDestroyPipelineLayout(deviceManager->logicalDevice, resources.pipelineLayout, nullptr);
	resources.pipelineLayout = NULL;

	vkDestroyDescriptorSetLayout(deviceManager->logicalDevice, resources.descriptorSetLayout, nullptr);
	resources.descriptorSetLayout = NULL;

	for (auto& fence : resources.drawFences) {
		vkDestroyFence(deviceManager->logicalDevice, fence, nullptr);
		fence = NULL;
	}

	vkDestroySemaphore(deviceManager->logicalDevice, resources.semaphorePresent, nullptr);
	resources.semaphorePresent = NULL;

	vkDestroySemaphore(deviceManager->logicalDevice, resources.semaphoreRender, nullptr);
	resources.semaphoreRender = NULL;

	for (auto& buffer : resources.frameBuffers) {
		vkDestroyFramebuffer(deviceManager->logicalDevice, buffer, nullptr);
		buffer = NULL;
	}

	vkDestroyPipelineCache(deviceManager->logicalDevice, resources.pipelineCache, nullptr);
	resources.pipelineCache = NULL;

	vkDestroyRenderPass(deviceManager->logicalDevice, resources.renderPass, nullptr);
	resources.renderPass = NULL;

	vkDestroyImageView(deviceManager->logicalDevice, resources.depthStencilImageView, nullptr);
	resources.depthStencilImageView = NULL;

	vkFreeMemory(deviceManager->logicalDevice, resources.depthStencilImageMemory, nullptr);
	resources.depthStencilImageMemory = NULL;

	vkDestroyImage(deviceManager->logicalDevice, resources.depthStencilImage, nullptr);
	resources.depthStencilImage = NULL;

	for (uint32_t i = 0; i < resources.drawCommandBuffers.size(); i++) {
		vkFreeCommandBuffers(deviceManager->logicalDevice, deviceManager->commandPool, 1, &resources.drawCommandBuffers[i]);
		resources.drawCommandBuffers[i] = NULL;
	}

	for (uint32_t i = 0; i < resources.swapchainImageViews.size(); i++) {
		vkDestroyImageView(deviceManager->logicalDevice, resources.swapchainImageViews[i], nullptr);
		resources.swapchainImageViews[i] = NULL;
	}

	vkDestroySwapchainKHR(deviceManager->logicalDevice, resources.swapchain, nullptr);
	resources.swapchain = VK_NULL_HANDLE;
}

void Renderer::CreateGraphicsQueue()
{
	vkGetDeviceQueue(deviceManager->logicalDevice, deviceManager->queueFamilyIndices.graphics, 0, &graphicsQueue);
}

void Renderer::SelectDepthFormat()
{
	std::vector<VkFormat> depthFormats = {
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};

	for (auto& format : depthFormats)
	{
		VkFormatProperties formatProps;
		vkGetPhysicalDeviceFormatProperties(deviceManager->physicalDevice, format, &formatProps);

		// Format must support depth stencil attachment for optimal tiling
		if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			resources.depthStencilFormat = format;
			return;
		}
	}

	throw std::runtime_error("failed to select depth format");
}

void Renderer::CreateSwapChain()
{
	VkSwapchainKHR oldSwapchain = resources.swapchain;

	uint32_t formatCount;
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(deviceManager->physicalDevice, surface, &formatCount, NULL) != VK_SUCCESS)
		throw std::runtime_error("Renderer::CreateSwapChain failed to vkGetPhysicalDeviceSurfaceFormatsKHR count");

	std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(deviceManager->physicalDevice, surface, &formatCount, surfaceFormats.data()) != VK_SUCCESS)
		throw std::runtime_error("Renderer::CreateSwapChain failed to vkGetPhysicalDeviceSurfaceFormatsKHR");

	if ((formatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
	{
		// If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
		// there is no preferered format, so we assume VK_FORMAT_B8G8R8A8_UNORM
		resources.swapchainColorFormat = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else
	{
		// Always select the first available color format
		// If you need a specific format (e.g. SRGB) you'd need to
		// iterate over the list of available surface format and
		// check for it's presence
		resources.swapchainColorFormat = surfaceFormats[0].format;
	}
	resources.swapchainColorSpace = surfaceFormats[0].colorSpace;

	std::vector<VkBool32> supportsPresent(formatCount);
	for (uint32_t i = 0; i < formatCount; i++)
	{
		if (vkGetPhysicalDeviceSurfaceSupportKHR(deviceManager->physicalDevice, i, surface, &supportsPresent[i]) != VK_SUCCESS)
			throw std::runtime_error("failed to get physicalDevice surface support");
	}

	// Get physical device surface properties and formats
	VkSurfaceCapabilitiesKHR surfCapabilities;
	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(deviceManager->physicalDevice, surface, &surfCapabilities) != VK_SUCCESS)
		throw std::runtime_error("Renderer::CreateSwapChain failed to vkGetPhysicalDeviceSurfaceCapabilitiesKHR");

	// Get available present modes
	uint32_t presentModeCount;
	if (vkGetPhysicalDeviceSurfacePresentModesKHR(deviceManager->physicalDevice, surface, &presentModeCount, NULL) != VK_SUCCESS)
		throw std::runtime_error("Renderer::CreateSwapChain failed to vkGetPhysicalDeviceSurfaceCapabilitiesKHR");

	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	if (vkGetPhysicalDeviceSurfacePresentModesKHR(deviceManager->physicalDevice, surface, &presentModeCount, presentModes.data()) != VK_SUCCESS)
		throw std::runtime_error("Renderer::CreateSwapChain failed to vkGetPhysicalDeviceSurfaceCapabilitiesKHR");

	VkExtent2D swapchainExtent = {};
	// If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
	if (surfCapabilities.currentExtent.width == (uint32_t)-1)
	{
		// If the surface size is undefined, the size is set to
		// the size of the images requested.
		swapchainExtent.width = width;
		swapchainExtent.height = height;
	}
	else
	{
		// If the surface size is defined, the swap chain size must match
		swapchainExtent = surfCapabilities.currentExtent;
		width = surfCapabilities.currentExtent.width;
		height = surfCapabilities.currentExtent.height;
	}

	// Select a present mode for the swapchain

	// The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
	// This mode waits for the vertical blank ("v-sync")
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

	// If v-sync is not requested, try to find a mailbox mode
	// It's the lowest latency non-tearing present mode available
	if (!verticalSync)
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
	uint32_t desiredNumberOfSwapchainImages = surfCapabilities.minImageCount + 1;
	if ((surfCapabilities.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfCapabilities.maxImageCount))
	{
		desiredNumberOfSwapchainImages = surfCapabilities.maxImageCount;
	}

	// Find the transformation of the surface
	VkSurfaceTransformFlagsKHR preTransform;
	if (surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		// We prefer a non-rotated transform
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		preTransform = surfCapabilities.currentTransform;
	}

	VkSwapchainCreateInfoKHR swapchainCI = {};
	swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCI.pNext = NULL;
	swapchainCI.surface = surface;
	swapchainCI.minImageCount = desiredNumberOfSwapchainImages;
	swapchainCI.imageFormat = resources.swapchainColorFormat;
	swapchainCI.imageColorSpace = resources.swapchainColorSpace;
	swapchainCI.imageExtent = { swapchainExtent.width, swapchainExtent.height };
	swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCI.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
	swapchainCI.imageArrayLayers = 1;
	swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCI.queueFamilyIndexCount = 0;
	swapchainCI.pQueueFamilyIndices = NULL;
	swapchainCI.presentMode = swapchainPresentMode;
	swapchainCI.oldSwapchain = oldSwapchain;
	// Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
	swapchainCI.clipped = VK_TRUE;
	swapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	// Set additional usage flag for blitting from the swapchain images if supported
	VkFormatProperties formatProps;
	vkGetPhysicalDeviceFormatProperties(deviceManager->physicalDevice, resources.swapchainColorFormat, &formatProps);
	if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT) {
		swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}

	if (vkCreateSwapchainKHR(deviceManager->logicalDevice, &swapchainCI, nullptr, &resources.swapchain) != VK_SUCCESS)
		throw std::runtime_error("Renderer::CreateSwapChain failed vkCreateSwapchainKHR");

	// If an existing swap chain is re-created, destroy the old swap chain
	// This also cleans up all the presentable images
	if (oldSwapchain != VK_NULL_HANDLE)
	{
		for (uint32_t i = 0; i < resources.swapchainImageCount; i++)
			vkDestroyImageView(deviceManager->logicalDevice, resources.swapchainImageViews[i], nullptr);

		vkDestroySwapchainKHR(deviceManager->logicalDevice, oldSwapchain, nullptr);
	}

	// get swap chain image count
	if (vkGetSwapchainImagesKHR(deviceManager->logicalDevice, resources.swapchain, &resources.swapchainImageCount, NULL) != VK_SUCCESS)
		throw std::runtime_error("Renderer::CreateSwapChain failed vkGetSwapchainImagesKHR");

	resources.swapchainImages.resize(resources.swapchainImageCount);
	resources.swapchainImageViews.resize(resources.swapchainImageCount);

	// Get the swap chain images
	if (vkGetSwapchainImagesKHR(deviceManager->logicalDevice, resources.swapchain, &resources.swapchainImageCount, resources.swapchainImages.data()) != VK_SUCCESS)
		throw std::runtime_error("Renderer::CreateSwapChain failed vkGetSwapchainImagesKHR");

	// Get the swap chain image and imageview
	for (uint32_t i = 0; i < resources.swapchainImageCount; i++)
	{
		VkImageViewCreateInfo colorAttachmentView = {};
		colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		colorAttachmentView.pNext = NULL;
		colorAttachmentView.format = resources.swapchainColorFormat;
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

		colorAttachmentView.image = resources.swapchainImages[i];

		if (vkCreateImageView(deviceManager->logicalDevice, &colorAttachmentView, nullptr, &resources.swapchainImageViews[i]) != VK_SUCCESS)
			throw std::runtime_error("Renderer::CreateSwapChain failed vkCreateImageView");
	}
}

void Renderer::CreateDrawCommandBuffers()
{
	resources.drawCommandBuffers.resize(resources.swapchainImageCount);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandPool = deviceManager->commandPool;
	commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(resources.drawCommandBuffers.size());

	if (vkAllocateCommandBuffers(deviceManager->logicalDevice, &commandBufferAllocateInfo, resources.drawCommandBuffers.data()) != VK_SUCCESS)
		throw std::runtime_error("failed to create draw command buffers");
}

void Renderer::CreateDepthStencil()
{
	SelectDepthFormat();

	// Create an optimal image used as the depth stencil attachment
	VkImageCreateInfo image = {};
	image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image.imageType = VK_IMAGE_TYPE_2D;
	image.format = resources.depthStencilFormat;
	// Use example's height and width
	image.extent = { (uint32_t)width, (uint32_t)height, 1 };
	image.mipLevels = 1;
	image.arrayLayers = 1;
	image.samples = VK_SAMPLE_COUNT_1_BIT;
	image.tiling = VK_IMAGE_TILING_OPTIMAL;
	image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	if (vkCreateImage(deviceManager->logicalDevice, &image, nullptr, &resources.depthStencilImage) != VK_SUCCESS)
		throw std::runtime_error("failed to create depth image");

	// Allocate memory for the image (device local) and bind it to our image
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(deviceManager->logicalDevice, resources.depthStencilImage, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = deviceManager->GetMemoryTypeIndex(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(deviceManager->logicalDevice, &memoryAllocateInfo, nullptr, &resources.depthStencilImageMemory) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate depth image memory");

	if (vkBindImageMemory(deviceManager->logicalDevice, resources.depthStencilImage, resources.depthStencilImageMemory, 0) != VK_SUCCESS)
		throw std::runtime_error("failed to bind depth image memory");

	// Create a view for the depth stencil image
	// Images aren't directly accessed in Vulkan, but rather through views described by a subresource range
	// This allows for multiple views of one image with differing ranges (e.g. for different layers)
	VkImageViewCreateInfo depthStencilImageViewCreateInfo = {};
	depthStencilImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depthStencilImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthStencilImageViewCreateInfo.format = resources.depthStencilFormat;
	depthStencilImageViewCreateInfo.subresourceRange = {};
	depthStencilImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	depthStencilImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	depthStencilImageViewCreateInfo.subresourceRange.levelCount = 1;
	depthStencilImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	depthStencilImageViewCreateInfo.subresourceRange.layerCount = 1;
	depthStencilImageViewCreateInfo.image = resources.depthStencilImage;

	if (vkCreateImageView(deviceManager->logicalDevice, &depthStencilImageViewCreateInfo, nullptr, &resources.depthStencilImageView) != VK_SUCCESS)
		throw std::runtime_error("failed to create depth image view");
}

void Renderer::CreateRenderPass()
{
	// This example will use a single render pass with one subpass

	// Descriptors for the attachments used by this renderpass
	std::array<VkAttachmentDescription, 2> attachments = {};

	// Color attachment
	attachments[0].format = VK_FORMAT_B8G8R8A8_UNORM;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;									// We don't use multi sampling in this example
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;							// Clear this attachment at the start of the render pass
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;							// Keep it's contents after the render pass is finished (for displaying it)
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;					// We don't use stencil, so don't care for load
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;				// Same for store
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;						// Layout at render pass start. Initial doesn't matter, so we use undefined
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;					// Layout to which the attachment is transitioned when the render pass is finished
																					// As we want to present the color buffer to the swapchain, we transition to PRESENT_KHR	
																					// Depth attachment
	attachments[1].format = resources.depthStencilFormat;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;							// Clear depth at start of first subpass
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;						// We don't need depth after render pass has finished (DONT_CARE may result in better performance)
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;					// No stencil
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;				// No Stencil
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;						// Layout at render pass start. Initial doesn't matter, so we use undefined
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;	// Transition to depth/stencil attachment

																					// Setup attachment references
	VkAttachmentReference colorReference = {};
	colorReference.attachment = 0;													// Attachment 0 is color
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;				// Attachment layout used as color during the subpass

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 1;													// Attachment 1 is color
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;		// Attachment used as depth/stemcil used during the subpass

																					// Setup a single subpass reference
	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;									// Subpass uses one color attachment
	subpassDescription.pColorAttachments = &colorReference;							// Reference to the color attachment in slot 0
	subpassDescription.pDepthStencilAttachment = &depthReference;					// Reference to the depth attachment in slot 1
	subpassDescription.inputAttachmentCount = 0;									// Input attachments can be used to sample from contents of a previous subpass
	subpassDescription.pInputAttachments = nullptr;									// (Input attachments not used by this example)
	subpassDescription.preserveAttachmentCount = 0;									// Preserved attachments can be used to loop (and preserve) attachments through subpasses
	subpassDescription.pPreserveAttachments = nullptr;								// (Preserve attachments not used by this example)
	subpassDescription.pResolveAttachments = nullptr;								// Resolve attachments are resolved at the end of a sub pass and can be used for e.g. multi sampling

																					// Setup subpass dependencies
																					// These will add the implicit ttachment layout transitionss specified by the attachment descriptions
																					// The actual usage layout is preserved through the layout specified in the attachment reference		
																					// Each subpass dependency will introduce a memory and execution dependency between the source and dest subpass described by
																					// srcStageMask, dstStageMask, srcAccessMask, dstAccessMask (and dependencyFlags is set)
																					// Note: VK_SUBPASS_EXTERNAL is a special constant that refers to all commands executed outside of the actual renderpass)
	std::array<VkSubpassDependency, 2> dependencies;

	// First dependency at the start of the renderpass
	// Does the transition from final to initial layout 
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;								// Producer of the dependency 
	dependencies[0].dstSubpass = 0;													// Consumer is our single subpass that will wait for the execution depdendency
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// Second dependency at the end the renderpass
	// Does the transition from the initial to the final layout
	dependencies[1].srcSubpass = 0;													// Producer of the dependency is our single subpass
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;								// Consumer are all commands outside of the renderpass
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// Create the actual renderpass
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());		// Number of attachments used by this render pass
	renderPassInfo.pAttachments = attachments.data();								// Descriptions of the attachments used by the render pass
	renderPassInfo.subpassCount = 1;												// We only use one subpass in this example
	renderPassInfo.pSubpasses = &subpassDescription;								// Description of that subpass
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());	// Number of subpass dependencies
	renderPassInfo.pDependencies = dependencies.data();								// Subpass dependencies used by the render pass

	if (vkCreateRenderPass(deviceManager->logicalDevice, &renderPassInfo, nullptr, &resources.renderPass) != VK_SUCCESS)
		throw std::runtime_error("failed to create render pass");
}

void Renderer::CreatePipelineCache()
{
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

	if (vkCreatePipelineCache(deviceManager->logicalDevice, &pipelineCacheCreateInfo, nullptr, &resources.pipelineCache) != VK_SUCCESS)
		throw std::runtime_error("failed to create pipeline cache");
}

void Renderer::CreateDrawFrameBuffers()
{
	// Create a frame buffer for every image in the swapchain
	resources.frameBuffers.resize(resources.swapchainImageCount);

	for (size_t i = 0; i < resources.frameBuffers.size(); i++)
	{
		std::array<VkImageView, 2> attachments;
		attachments[0] = resources.swapchainImageViews[i];				// Color attachment is the view of the swapchain image			
		attachments[1] = resources.depthStencilImageView;				// Depth/Stencil attachment is the same for all frame buffers			

		VkFramebufferCreateInfo frameBufferCreateInfo = {};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		// All frame buffers use the same renderpass setup
		frameBufferCreateInfo.renderPass = resources.renderPass;
		frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		frameBufferCreateInfo.pAttachments = attachments.data();
		frameBufferCreateInfo.width = width;
		frameBufferCreateInfo.height = height;
		frameBufferCreateInfo.layers = 1;

		// Create the framebuffer
		if (vkCreateFramebuffer(deviceManager->logicalDevice, &frameBufferCreateInfo, nullptr, &resources.frameBuffers[i]) != VK_SUCCESS)
			throw std::runtime_error("failed to create frame buffer");
	}
}

void Renderer::CreateSemaphores()
{
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = NULL;
	semaphoreCreateInfo.flags = 0;

	if (vkCreateSemaphore(deviceManager->logicalDevice, &semaphoreCreateInfo, nullptr, &resources.semaphorePresent) != VK_SUCCESS)
		throw std::runtime_error("failed to create image presentation semaphore");

	if (vkCreateSemaphore(deviceManager->logicalDevice, &semaphoreCreateInfo, nullptr, &resources.semaphoreRender) != VK_SUCCESS)
		throw std::runtime_error("failed to create command submission semaphore");

	resources.drawFences.resize(resources.drawCommandBuffers.size());

	// Fences (Used to check draw command buffer completion)
	// Create in signaled state so we don't wait on first render of each command buffer
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (auto& fence : resources.drawFences)
		if (vkCreateFence(deviceManager->logicalDevice, &fenceCreateInfo, nullptr, &fence) != VK_SUCCESS)
			throw std::runtime_error("failed to create fence");
}

void Renderer::CreateDescriptorSetLayout()
{
	// Setup layout of descriptors used in this example
	// Basically connects the different shader stages to descriptors for binding uniform buffers, image samplers, etc.
	// So every shader binding should map to one descriptor set layout binding

	// Binding 0: Uniform buffer (Vertex shader)
	VkDescriptorSetLayoutBinding layoutBinding = {};
	layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBinding.descriptorCount = 1;
	layoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
	descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayout.pNext = nullptr;
	descriptorLayout.bindingCount = 1;
	descriptorLayout.pBindings = &layoutBinding;

	if (vkCreateDescriptorSetLayout(deviceManager->logicalDevice, &descriptorLayout, nullptr, &resources.descriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("failed to alloc descriptior set layout");
}

void Renderer::CreatePipelineLayout()
{
	// Create the pipeline layout that is used to generate the rendering pipelines that are based on this descriptor set layout
	// In a more complex scenario you would have different pipeline layouts for different descriptor set layouts that could be reused
	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
	pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pPipelineLayoutCreateInfo.pNext = nullptr;
	pPipelineLayoutCreateInfo.setLayoutCount = 1;
	pPipelineLayoutCreateInfo.pSetLayouts = &resources.descriptorSetLayout;

	if (vkCreatePipelineLayout(deviceManager->logicalDevice, &pPipelineLayoutCreateInfo, nullptr, &resources.pipelineLayout) != VK_SUCCESS)
		throw std::runtime_error("failed to create pipeline layout");
}

void Renderer::CreatePipelineVertexInputState()
{
	struct Vertex
	{
		float position[3];
		float color[3];
	};

	// Vertex input binding
	resources.vertexInputBindingDescription.binding = 0;
	resources.vertexInputBindingDescription.stride = sizeof(Vertex);
	resources.vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	// Inpute attribute binding describe shader attribute locations and memory layouts
	// These match the following shader layout (see triangle.vert):
	//	layout (location = 0) in vec3 inPos;
	//	layout (location = 1) in vec3 inColor;
	resources.vertexInputAttributeDescriptions.resize(2);

	// Attribute location 0: Position
	resources.vertexInputAttributeDescriptions[0].binding = 0;
	resources.vertexInputAttributeDescriptions[0].location = 0;
	resources.vertexInputAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	resources.vertexInputAttributeDescriptions[0].offset = offsetof(Vertex, position);

	// Attribute location 1: Color
	resources.vertexInputAttributeDescriptions[1].binding = 0;
	resources.vertexInputAttributeDescriptions[1].location = 1;
	resources.vertexInputAttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	resources.vertexInputAttributeDescriptions[1].offset = offsetof(Vertex, color);

	// Assign to the vertex input state used for pipeline creation
	resources.pipelineVertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	resources.pipelineVertexInputState.pNext = nullptr;
	resources.pipelineVertexInputState.flags = 0;
	resources.pipelineVertexInputState.vertexBindingDescriptionCount = 1;
	resources.pipelineVertexInputState.pVertexBindingDescriptions = &resources.vertexInputBindingDescription;
	resources.pipelineVertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(resources.vertexInputAttributeDescriptions.size());
	resources.pipelineVertexInputState.pVertexAttributeDescriptions = resources.vertexInputAttributeDescriptions.data();
}

void Renderer::CreateDescriptorPool()
{
	// We need to tell the API the number of max. requested descriptors per type
	VkDescriptorPoolSize typeCounts[1];
	// This example only uses one descriptor type (uniform buffer) and only requests one descriptor of this type
	typeCounts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	typeCounts[0].descriptorCount = 1;

	// For additional types you need to add new entries in the type count list
	// E.g. for two combined image samplers :
	// typeCounts[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	// typeCounts[1].descriptorCount = 2;

	// Create the global descriptor pool
	// All descriptors used in this example are allocated from this pool
	VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.pNext = nullptr;
	descriptorPoolInfo.poolSizeCount = 1;
	descriptorPoolInfo.pPoolSizes = typeCounts;
	// Set the max. number of descriptor sets that can be requested from this pool (requesting beyond this limit will result in an error)
	descriptorPoolInfo.maxSets = 1;

	if (vkCreateDescriptorPool(deviceManager->logicalDevice, &descriptorPoolInfo, nullptr, &resources.descriptorPool) != VK_SUCCESS)
		throw std::runtime_error("failed to create descriptior set");
}

void Renderer::CreateDescriptorSet()
{
	// Allocate a new descriptor set from the global descriptor pool
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = resources.descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &resources.descriptorSetLayout;

	if (vkAllocateDescriptorSets(deviceManager->logicalDevice, &allocInfo, &resources.descriptorSet) != VK_SUCCESS)
		throw std::runtime_error("failed to alloc descriptior set");
}