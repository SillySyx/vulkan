#include "HudManager.h"

HudManager::~HudManager()
{
	if (!renderer || !renderer->logicalDevice)
		return;

	vkFreeCommandBuffers(renderer->logicalDevice, commandPool, (uint32_t)cmdBuffers.size(), cmdBuffers.data());
	cmdBuffers.clear();

	DestroyPipeline();
	DestroyRenderPass();
	DestroyFences();
	DestroyPipelineCache();
	DestroyPipelineLayout();
	DestroyDescriptorSetLayout();
	DestroyDescriptorPool();
	DestroyFontSampler();
	DestroyFontTexture();
	DestroyVertexBuffer();
	DestroyCommandPool();
}

void HudManager::Initiate(void * renderer, void * contentManager)
{
	logger.Info("HudManager::Initiate ...");

	this->renderer = (RendererForward*)renderer;
	this->contentManager = (ContentManager*)contentManager;

	cmdBuffers.resize(this->renderer->frameBuffers.size());

	InitCommandPool();
	InitVertexBuffer();
	InitFontTexture();
	InitFontSampler();
	InitDescriptorPool();
	InitDescriptorSetLayout();
	InitPipelineLayout();
	InitPipelineCache();
	InitFences();
	InitRenderPass();
	InitPipeline();

	logger.Info("HudManager::Initiate done");
}

void HudManager::Render()
{
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = (uint32_t)cmdBuffers.size();
	submitInfo.pCommandBuffers = cmdBuffers.data();

	if (vkQueueSubmit(renderer->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
		throw "Failed to submit queue";

	if (vkWaitForFences(renderer->logicalDevice, 1, &fence, VK_TRUE, UINT64_MAX) != VK_SUCCESS)
		throw "Failed to wait for fences";
	
	if (vkResetFences(renderer->logicalDevice, 1, &fence) != VK_SUCCESS)
		throw "Failed to reset fences";
}

void HudManager::BeginTextUpdate()
{
	mappedTextData = (glm::vec4*)mappedVertexBuffer;
	numLetters = 0;
}

void HudManager::EndTextUpdate()
{
	UpdateCommandBuffers();
}

void HudManager::AddText(std::string text, float x, float y, TextAlign align)
{
	if (align == Left) {
		x *= scale;
	};

	y *= scale;

	const float charW = (1.5f * scale) / renderer->width;
	const float charH = (1.5f * scale) / renderer->height;

	float fbW = (float)renderer->width;
	float fbH = (float)renderer->height;
	x = (x / fbW * 2.0f) - 1.0f;
	y = (y / fbH * 2.0f) - 1.0f;

	// Calculate text width
	float textWidth = 0;
	for (auto letter : text)
	{
		stb_fontchar *charData = &stbFontData[(uint32_t)letter - STB_FIRST_CHAR];
		textWidth += charData->advance * charW;
	}

	switch (align)
	{
	case Right:
		x -= textWidth;
		break;
	case Center:
		x -= textWidth / 2.0f;
		break;
	case Left:
		break;
	}

	// Generate a uv mapped quad per char in the new text
	for (auto letter : text)
	{
		stb_fontchar *charData = &stbFontData[(uint32_t)letter - STB_FIRST_CHAR];

		mappedTextData->x = (x + (float)charData->x0 * charW);
		mappedTextData->y = (y + (float)charData->y0 * charH);
		mappedTextData->z = charData->s0;
		mappedTextData->w = charData->t0;
		mappedTextData++;

		mappedTextData->x = (x + (float)charData->x1 * charW);
		mappedTextData->y = (y + (float)charData->y0 * charH);
		mappedTextData->z = charData->s1;
		mappedTextData->w = charData->t0;
		mappedTextData++;

		mappedTextData->x = (x + (float)charData->x0 * charW);
		mappedTextData->y = (y + (float)charData->y1 * charH);
		mappedTextData->z = charData->s0;
		mappedTextData->w = charData->t1;
		mappedTextData++;

		mappedTextData->x = (x + (float)charData->x1 * charW);
		mappedTextData->y = (y + (float)charData->y1 * charH);
		mappedTextData->z = charData->s1;
		mappedTextData->w = charData->t1;
		mappedTextData++;

		x += charData->advance * charW;

		numLetters++;
	}
}

void HudManager::InitCommandPool()
{
	logger.Info("HudManager::InitCommandPool ...");

	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = renderer->graphicsQueueFamilyIndex;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(renderer->logicalDevice, &cmdPoolInfo, nullptr, &commandPool) != VK_SUCCESS)
		throw "Failed to create command pool";

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = (uint32_t)cmdBuffers.size();

	if (vkAllocateCommandBuffers(renderer->logicalDevice, &commandBufferAllocateInfo, cmdBuffers.data()) != VK_SUCCESS)
		throw "Failed to allocate command buffers";

	logger.Info("HudManager::InitCommandPool command pool " + logger.GetAddr(commandPool));
}

void HudManager::DestroyCommandPool()
{
	if (commandPool)
	{
		logger.Info("HudManager::DestroyCommandPool " + logger.GetAddr(commandPool));

		vkDestroyCommandPool(renderer->logicalDevice, commandPool, nullptr);
		commandPool = nullptr;

		logger.Info("HudManager::DestroyCommandPool done");
	}
}

void HudManager::InitVertexBuffer()
{
	logger.Info("HudManager::InitVertexBuffer ...");

	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = NULL;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferCreateInfo.size = MAX_CHAR_COUNT * sizeof(glm::vec4);
	bufferCreateInfo.flags = 0;

	if (vkCreateBuffer(renderer->logicalDevice, &bufferCreateInfo, nullptr, &vertexBuffer) != VK_SUCCESS)
		throw "Failed to create vertex buffer";

	logger.Info("HudManager::InitVertexBuffer buffer " + logger.GetAddr(vertexBuffer));

	VkMemoryAllocateInfo memAllocInfo = {};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.pNext = NULL;
	memAllocInfo.allocationSize = 0;
	memAllocInfo.memoryTypeIndex = 0;

	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(renderer->logicalDevice, vertexBuffer, &memReqs);

	memAllocInfo.allocationSize = memReqs.size;
	// Find a memory type index that fits the properties of the buffer
	memAllocInfo.memoryTypeIndex = renderer->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (vkAllocateMemory(renderer->logicalDevice, &memAllocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS)
		throw "Failed to allocate vertex buffer memory";

	if (vkBindBufferMemory(renderer->logicalDevice, vertexBuffer, vertexBufferMemory, 0) != VK_SUCCESS)
		throw "failed to bind vertex buffer memory";

	if (vkMapMemory(renderer->logicalDevice, vertexBufferMemory, 0, bufferCreateInfo.size, 0, &mappedVertexBuffer) != VK_SUCCESS)
		throw "failed to map vertex buffer memory";

	logger.Info("HudManager::InitVertexBuffer memory " + logger.GetAddr(vertexBufferMemory));
}

void HudManager::DestroyVertexBuffer()
{
	if (vertexBufferMemory)
	{
		logger.Info("HudManager::DestroyVertexBuffer memory " + logger.GetAddr(vertexBufferMemory));

		vkUnmapMemory(renderer->logicalDevice, vertexBufferMemory);
		vkFreeMemory(renderer->logicalDevice, vertexBufferMemory, nullptr);
		vertexBufferMemory = nullptr;

		logger.Info("HudManager::DestroyVertexBuffer memory done");
	}
	if (vertexBuffer)
	{
		logger.Info("HudManager::DestroyVertexBuffer buffer " + logger.GetAddr(vertexBuffer));

		vkDestroyBuffer(renderer->logicalDevice, vertexBuffer, nullptr);
		vertexBuffer = nullptr;

		logger.Info("HudManager::DestroyVertexBuffer buffer done");
	}
}

void HudManager::InitFontTexture()
{
	logger.Info("HudManager::InitFontTexture ...");

	static unsigned char font24pixels[STB_FONT_HEIGHT][STB_FONT_WIDTH];
	STB_FONT_NAME(stbFontData, font24pixels, STB_FONT_HEIGHT);

	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.pNext = NULL;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = VK_FORMAT_R8_UNORM;
	imageInfo.extent.width = STB_FONT_WIDTH;
	imageInfo.extent.height = STB_FONT_HEIGHT;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

	if (vkCreateImage(renderer->logicalDevice, &imageInfo, nullptr, &fontImage) != VK_SUCCESS)
		throw "Failed to create font texture";

	logger.Info("HudManager::InitFontTexture image " + logger.GetAddr(fontImage));

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = NULL;
	allocInfo.allocationSize = 0;
	allocInfo.memoryTypeIndex = 0;

	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(renderer->logicalDevice, fontImage, &memReqs);

	allocInfo.allocationSize = memReqs.size;
	allocInfo.memoryTypeIndex = renderer->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if (vkAllocateMemory(renderer->logicalDevice, &allocInfo, nullptr, &fontImageMemory) != VK_SUCCESS)
		throw "failed to allocate font image memory";

	if (vkBindImageMemory(renderer->logicalDevice, fontImage, fontImageMemory, 0) != VK_SUCCESS)
		throw "failed to bind font image memory";

	logger.Info("HudManager::InitFontTexture memory " + logger.GetAddr(fontImageMemory));

	// Staging

	auto size = allocInfo.allocationSize;

	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.pNext = NULL;
	bufferCreateInfo.size = size;
	bufferCreateInfo.flags = 0;

	VkBuffer stagingBuffer;
	if (vkCreateBuffer(renderer->logicalDevice, &bufferCreateInfo, nullptr, &stagingBuffer) != VK_SUCCESS)
		throw "Failed to create staging buffer";

	// Create the memory backing up the buffer handle
	VkMemoryAllocateInfo memAllocInfo = {};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.pNext = NULL;
	memAllocInfo.allocationSize = 0;
	memAllocInfo.memoryTypeIndex = 0;

	memReqs = {};
	vkGetBufferMemoryRequirements(renderer->logicalDevice, stagingBuffer, &memReqs);
	memAllocInfo.allocationSize = memReqs.size;
	// Find a memory type index that fits the properties of the buffer
	memAllocInfo.memoryTypeIndex = renderer->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VkDeviceMemory stagingBufferMemory;
	if (vkAllocateMemory(renderer->logicalDevice, &memAllocInfo, nullptr, &stagingBufferMemory) != VK_SUCCESS)
		throw "failed to allocate staging buffer memory";

	// Initialize a default descriptor that covers the whole buffer size
	VkDescriptorBufferInfo descriptor;
	descriptor.offset = 0;
	descriptor.buffer = stagingBuffer;
	descriptor.range = VK_WHOLE_SIZE;

	// Attach the memory to the buffer object
	if (vkBindBufferMemory(renderer->logicalDevice, stagingBuffer, stagingBufferMemory, 0) != VK_SUCCESS)
		throw "failed to bind staging buffer memory";

	void * mapped;
	if (vkMapMemory(renderer->logicalDevice, stagingBufferMemory, 0, VK_WHOLE_SIZE, 0, &mapped) != VK_SUCCESS)
		throw "Failed to map staing stating buffer memory";

	memcpy(mapped, &font24pixels[0][0], STB_FONT_WIDTH * STB_FONT_HEIGHT);	// Only one channel, so data size = W * H (*R8)

	vkUnmapMemory(renderer->logicalDevice, stagingBufferMemory);
	mapped = nullptr;




	// Copy to image
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.commandBufferCount = 1;

	VkCommandBuffer copyCommandBuffer;
	if (vkAllocateCommandBuffers(renderer->logicalDevice, &commandBufferAllocateInfo, &copyCommandBuffer) != VK_SUCCESS)
		throw "Failed to allocate copy image data command buffer";

	VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufferBeginInfo.pNext = NULL;
	if (vkBeginCommandBuffer(copyCommandBuffer, &cmdBufferBeginInfo) != VK_SUCCESS)
		throw "Failed to begin copy image data command buffer";

	// Prepare for transfer
	{
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 1;

		VkImageMemoryBarrier imageMemoryBarrier = {};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.pNext = NULL;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageMemoryBarrier.image = fontImage;
		imageMemoryBarrier.subresourceRange = subresourceRange;

		// Image is preinitialized
		// Only valid as initial layout for linear images, preserves memory contents
		// Make sure host writes have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;

		// Image will be used as a transfer destination
		// Make sure any writes to the image have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = 0;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = STB_FONT_WIDTH;
		bufferCopyRegion.imageExtent.height = STB_FONT_HEIGHT;
		bufferCopyRegion.imageExtent.depth = 1;

		vkCmdPipelineBarrier(copyCommandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

		vkCmdCopyBufferToImage(copyCommandBuffer, stagingBuffer, fontImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferCopyRegion);
	}

	// Prepare for shader read
	{
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 1;

		VkImageMemoryBarrier imageMemoryBarrier = {};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageMemoryBarrier.image = fontImage;
		imageMemoryBarrier.subresourceRange = subresourceRange;

		vkCmdPipelineBarrier(copyCommandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
	}

	if (vkEndCommandBuffer(copyCommandBuffer) != VK_SUCCESS)
		throw "failed to end command buffer";

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &copyCommandBuffer;

	if (vkQueueSubmit(renderer->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
		throw "failed to submit queue";

	if (vkQueueWaitIdle(renderer->graphicsQueue) != VK_SUCCESS)
		throw "failed to wait for queue";

	vkDestroyBuffer(renderer->logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(renderer->logicalDevice, stagingBufferMemory, nullptr);

	vkFreeCommandBuffers(renderer->logicalDevice, commandPool, 1, &copyCommandBuffer);

	VkImageViewCreateInfo imageViewInfo = {};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.image = fontImage;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = imageInfo.format;
	imageViewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	imageViewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

	if (vkCreateImageView(renderer->logicalDevice, &imageViewInfo, nullptr, &fontImageView) != VK_SUCCESS)
		throw "failed to create image view";

	logger.Info("HudManager::InitFontTexture imageView " + logger.GetAddr(fontImageView));
}

void HudManager::DestroyFontTexture()
{
	if (fontImageView)
	{
		logger.Info("HudManager::DestroyFontTexture fontImageView " + logger.GetAddr(fontImageView));

		vkDestroyImageView(renderer->logicalDevice, fontImageView, nullptr);
		fontImageView = nullptr;

		logger.Info("HudManager::DestroyFontTexture fontImageView done");
	}
	if (fontImageMemory)
	{
		logger.Info("HudManager::DestroyFontTexture fontImageMemory " + logger.GetAddr(fontImageMemory));

		vkFreeMemory(renderer->logicalDevice, fontImageMemory, nullptr);
		fontImageMemory = nullptr;

		logger.Info("HudManager::DestroyFontTexture fontImageMemory done");
	}
	if (fontImage)
	{
		logger.Info("HudManager::DestroyFontTexture fontImage " + logger.GetAddr(fontImage));

		vkDestroyImage(renderer->logicalDevice, fontImage, nullptr);
		fontImage = nullptr;

		logger.Info("HudManager::DestroyFontTexture fontImage done");
	}
}

void HudManager::InitFontSampler()
{
	logger.Info("HudManager::InitFontSampler ...");

	// Sampler
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.maxAnisotropy = 1.0f;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 1.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

	if (vkCreateSampler(renderer->logicalDevice, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
		throw "failed to create sampler";

	logger.Info("HudManager::InitFontSampler " + logger.GetAddr(sampler));
}

void HudManager::DestroyFontSampler()
{
	if (sampler)
	{
		logger.Info("HudManager::DestroyFontSampler " + logger.GetAddr(sampler));

		vkDestroySampler(renderer->logicalDevice, sampler, nullptr);
		sampler = nullptr;

		logger.Info("HudManager::DestroyFontSampler done");
	}
}

void HudManager::InitDescriptorPool()
{
	logger.Info("HudManager::InitDescriptorPool ...");

	// Descriptor
	// Font uses a separate descriptor pool

	VkDescriptorPoolSize descriptorPoolSize{};
	descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorPoolSize.descriptorCount = 1;

	std::array<VkDescriptorPoolSize, 1> poolSizes;
	poolSizes[0] = descriptorPoolSize;

	VkDescriptorPoolCreateInfo descriptorPoolInfo{};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	descriptorPoolInfo.pPoolSizes = poolSizes.data();
	descriptorPoolInfo.maxSets = 1;

	if (vkCreateDescriptorPool(renderer->logicalDevice, &descriptorPoolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		throw "failed to create descriptor pool";

	logger.Info("HudManager::InitDescriptorPool " + logger.GetAddr(descriptorPool));
}

void HudManager::DestroyDescriptorPool()
{
	if (descriptorPool)
	{
		logger.Info("HudManager::DestroyDescriptorPool " + logger.GetAddr(descriptorPool));

		vkDestroyDescriptorPool(renderer->logicalDevice, descriptorPool, nullptr);
		descriptorPool = nullptr;

		logger.Info("HudManager::DestroyDescriptorPool done");
	}
}

void HudManager::InitDescriptorSetLayout()
{
	logger.Info("HudManager::InitDescriptorSetLayout ...");

	// Descriptor set layout
	VkDescriptorSetLayoutBinding setLayoutBinding{};
	setLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	setLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	setLayoutBinding.binding = 0;
	setLayoutBinding.descriptorCount = 1;

	std::array<VkDescriptorSetLayoutBinding, 1> setLayoutBindings;
	setLayoutBindings[0] = setLayoutBinding;

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.pBindings = setLayoutBindings.data();
	descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());

	if (vkCreateDescriptorSetLayout(renderer->logicalDevice, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		throw "failed to create descriptor set layout";

	logger.Info("HudManager::InitDescriptorSetLayout " + logger.GetAddr(descriptorSetLayout));

	// Descriptor set
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool;
	descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;
	descriptorSetAllocateInfo.descriptorSetCount = 1;

	if (vkAllocateDescriptorSets(renderer->logicalDevice, &descriptorSetAllocateInfo, &descriptorSet) != VK_SUCCESS)
		throw "failed to allocate descriptor sets";

	VkDescriptorImageInfo descriptorImageInfo{};
	descriptorImageInfo.sampler = sampler;
	descriptorImageInfo.imageView = fontImageView;
	descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkWriteDescriptorSet writeDescriptorSet{};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.dstSet = descriptorSet;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescriptorSet.dstBinding = 0;
	writeDescriptorSet.pImageInfo = &descriptorImageInfo;
	writeDescriptorSet.descriptorCount = 1;

	std::array<VkWriteDescriptorSet, 1> writeDescriptorSets;
	writeDescriptorSets[0] = writeDescriptorSet;

	vkUpdateDescriptorSets(renderer->logicalDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
}

void HudManager::DestroyDescriptorSetLayout()
{
	if (descriptorSetLayout)
	{
		logger.Info("HudManager::DestroyDescriptorSetLayout " + logger.GetAddr(descriptorSetLayout));

		vkDestroyDescriptorSetLayout(renderer->logicalDevice, descriptorSetLayout, nullptr);
		descriptorSetLayout = nullptr;

		logger.Info("HudManager::DestroyDescriptorSetLayout done");
	}
}

void HudManager::InitPipelineLayout()
{
	logger.Info("HudManager::InitPipelineLayout ...");

	// Pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;

	if (vkCreatePipelineLayout(renderer->logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		throw "failed to create pipeline layout";

	logger.Info("HudManager::InitPipelineLayout " + logger.GetAddr(pipelineLayout));
}

void HudManager::DestroyPipelineLayout()
{
	if (pipelineLayout)
	{
		logger.Info("HudManager::DestroyPipelineLayout " + logger.GetAddr(pipelineLayout));

		vkDestroyPipelineLayout(renderer->logicalDevice, pipelineLayout, nullptr);
		pipelineLayout = nullptr;

		logger.Info("HudManager::DestroyPipelineLayout done");
	}
}

void HudManager::InitPipelineCache()
{
	logger.Info("HudManager::InitPipelineCache ...");

	// Pipeline cache
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

	if (vkCreatePipelineCache(renderer->logicalDevice, &pipelineCacheCreateInfo, nullptr, &pipelineCache) != VK_SUCCESS)
		throw "failed to create pipeline cache";

	logger.Info("HudManager::InitPipelineCache " + logger.GetAddr(pipelineCache));
}

void HudManager::DestroyPipelineCache()
{
	if (pipelineCache)
	{
		logger.Info("HudManager::DestroyPipelineCache " + logger.GetAddr(pipelineCache));

		vkDestroyPipelineCache(renderer->logicalDevice, pipelineCache, nullptr);
		pipelineCache = nullptr;

		logger.Info("HudManager::DestroyPipelineCache done");
	}
}

void HudManager::InitFences()
{
	logger.Info("HudManager::InitFences ...");

	// Command buffer execution fence
	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = 0;

	if (vkCreateFence(renderer->logicalDevice, &fenceCreateInfo, nullptr, &fence) != VK_SUCCESS)
		throw "failed to create fences";

	logger.Info("HudManager::InitFences " + logger.GetAddr(fence));
}

void HudManager::DestroyFences()
{
	if (fence)
	{
		logger.Info("HudManager::DestroyFences " + logger.GetAddr(fence));

		vkDestroyFence(renderer->logicalDevice, fence, nullptr);
		fence = nullptr;

		logger.Info("HudManager::DestroyFences done");
	}
}

void HudManager::InitRenderPass()
{
	logger.Info("HudManager::InitRenderPass ...");

	VkAttachmentDescription attachments[2] = {};

	// Color attachment
	attachments[0].format = renderer->surfaceColorFormat;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	// Don't clear the framebuffer (like the renderpass from the example does)
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// Depth attachment
	attachments[1].format = renderer->depthFormat;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
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

	VkSubpassDependency subpassDependencies[2] = {};

	// Transition from final to initial (VK_SUBPASS_EXTERNAL refers to all commmands executed outside of the actual renderpass)
	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// Transition from initial to final
	subpassDependencies[1].srcSubpass = 0;
	subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.flags = 0;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = NULL;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorReference;
	subpassDescription.pResolveAttachments = NULL;
	subpassDescription.pDepthStencilAttachment = &depthReference;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = NULL;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pNext = NULL;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = subpassDependencies;

	if (vkCreateRenderPass(renderer->logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
		throw "failed to create render pass";

	logger.Info("HudManager::InitRenderPass " + logger.GetAddr(renderPass));
}

void HudManager::DestroyRenderPass()
{
	if (renderPass)
	{
		logger.Info("HudManager::DestroyRenderPass " + logger.GetAddr(renderPass));

		vkDestroyRenderPass(renderer->logicalDevice, renderPass, nullptr);
		renderPass = nullptr;

		logger.Info("HudManager::DestroyRenderPass done");
	}
}

void HudManager::InitPipeline()
{
	logger.Info("HudManager::InitPipeline ...");

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
	inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	inputAssemblyState.flags = 0;
	inputAssemblyState.primitiveRestartEnable = VK_FALSE;

	VkPipelineRasterizationStateCreateInfo rasterizationState{};
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizationState.flags = 0;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.lineWidth = 1.0f;

	// Enable blending
	VkPipelineColorBlendAttachmentState blendAttachmentState{};
	blendAttachmentState.colorWriteMask = 0xf;
	blendAttachmentState.blendEnable = VK_TRUE;
	blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
	blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
	blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo colorBlendState{};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &blendAttachmentState;

	VkPipelineDepthStencilStateCreateInfo depthStencilState{};
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.depthTestEnable = VK_FALSE;
	depthStencilState.depthWriteEnable = VK_FALSE;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilState.front = depthStencilState.back;
	depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
	viewportState.flags = 0;

	VkPipelineMultisampleStateCreateInfo multisampleState{};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleState.flags = 0;

	std::vector<VkDynamicState> dynamicStateEnables = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates = dynamicStateEnables.data();
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
	dynamicState.flags = 0;

	std::array<VkVertexInputBindingDescription, 2> vertexBindings = {};

	VkVertexInputBindingDescription vInputBindDescription{};
	vInputBindDescription.binding = 0;
	vInputBindDescription.stride = sizeof(glm::vec4);
	vInputBindDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	vertexBindings[0] = vInputBindDescription;

	vInputBindDescription.binding = 1;
	vInputBindDescription.stride = sizeof(glm::vec4);
	vInputBindDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	vertexBindings[1] = vInputBindDescription;

	std::array<VkVertexInputAttributeDescription, 2> vertexAttribs = {};

	// Position
	VkVertexInputAttributeDescription vInputAttribDescription{};
	vInputAttribDescription.location = 0;
	vInputAttribDescription.binding = 0;
	vInputAttribDescription.format = VK_FORMAT_R32G32_SFLOAT;
	vInputAttribDescription.offset = 0;
	vertexAttribs[0] = vInputAttribDescription;

	// UV
	vInputAttribDescription.location = 1;
	vInputAttribDescription.binding = 1;
	vInputAttribDescription.format = VK_FORMAT_R32G32_SFLOAT;
	vInputAttribDescription.offset = sizeof(glm::vec2);
	vertexAttribs[1] = vInputAttribDescription;

	VkPipelineVertexInputStateCreateInfo inputState{};
	inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindings.size());
	inputState.pVertexBindingDescriptions = vertexBindings.data();
	inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttribs.size());
	inputState.pVertexAttributeDescriptions = vertexAttribs.data();

	//std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	//shaderStages.resize(2);

	// Vertex shader
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].pNext = NULL;
	shaderStages[0].pSpecializationInfo = NULL;
	shaderStages[0].flags = 0;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = contentManager->LoadShaderSPIRV(renderer->logicalDevice, "src/Shaders/text.vert.spv");
	shaderStages[0].pName = "main";

	// Fragment shader
	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].pNext = NULL;
	shaderStages[1].pSpecializationInfo = NULL;
	shaderStages[1].flags = 0;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = contentManager->LoadShaderSPIRV(renderer->logicalDevice, "src/Shaders/text.frag.spv");
	shaderStages[1].pName = "main";

	VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.pNext = NULL;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = 0;
	pipelineCreateInfo.flags = 0;
	pipelineCreateInfo.pVertexInputState = &inputState;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pTessellationState = NULL;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pDynamicState = &dynamicState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.pStages = shaderStages;
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.subpass = 0;

	if (vkCreateGraphicsPipelines(renderer->logicalDevice, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS)
		throw "failed to create graphics pipelines";

	for (auto shaderStage : shaderStages)
	{
		vkDestroyShaderModule(renderer->logicalDevice, shaderStage.module, nullptr);
		shaderStage.module = VK_NULL_HANDLE;
	}

	logger.Info("HudManager::InitPipeline " + logger.GetAddr(pipeline));
}

void HudManager::DestroyPipeline()
{
	if (pipeline)
	{
		logger.Info("HudManager::DestroyPipeline " + logger.GetAddr(pipeline));

		vkDestroyPipeline(renderer->logicalDevice, pipeline, nullptr);
		pipeline = nullptr;

		logger.Info("HudManager::DestroyPipeline done");
	}
}

void HudManager::UpdateCommandBuffers()
{
	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.renderArea.extent.width = renderer->width;
	renderPassBeginInfo.renderArea.extent.height = renderer->height;
	renderPassBeginInfo.clearValueCount = 0;
	renderPassBeginInfo.pClearValues = nullptr;

	VkCommandBufferBeginInfo cmdBufInfo{};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	for (size_t i = 0; i < cmdBuffers.size(); ++i)
	{
		renderPassBeginInfo.framebuffer = renderer->frameBuffers[i];

		if (vkBeginCommandBuffer(cmdBuffers[i], &cmdBufInfo) != VK_SUCCESS)
			throw "Failed to begin command buffer";

		vkCmdBeginRenderPass(cmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.width = (float)renderer->width;
		viewport.height = (float)renderer->height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		vkCmdSetViewport(cmdBuffers[i], 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.extent.width = renderer->width;
		scissor.extent.height = renderer->height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;

		vkCmdSetScissor(cmdBuffers[i], 0, 1, &scissor);

		vkCmdBindPipeline(cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		vkCmdBindDescriptorSets(cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

		VkDeviceSize offsets = 0;
		vkCmdBindVertexBuffers(cmdBuffers[i], 0, 1, &vertexBuffer, &offsets);

		for (uint32_t j = 0; j < numLetters; j++)
			vkCmdDraw(cmdBuffers[i], 4, 1, j * 4, 0);

		vkCmdEndRenderPass(cmdBuffers[i]);

		if (vkEndCommandBuffer(cmdBuffers[i]) != VK_SUCCESS)
			throw "Failed to end command buffer";
	}
}