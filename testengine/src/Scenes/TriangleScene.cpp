#include "TriangleScene.h"

TriangleScene::~TriangleScene()
{
	if (!renderer || !renderer->logicalDevice)
		return;

	vkDeviceWaitIdle(renderer->logicalDevice);

	if (descriptorSet)
	{
		logger.Info("TriangleScene::~TriangleScene FreeDescriptorSets " + logger.GetAddr(descriptorSet));

		vkFreeDescriptorSets(renderer->logicalDevice, descriptorPool, 1, &descriptorSet);
		descriptorSet = nullptr;

		logger.Info("TriangleScene::~TriangleScene FreeDescriptorSets done");
	}

	if (descriptorPool)
	{
		logger.Info("TriangleScene::~TriangleScene DestroyDescriptorPool " + logger.GetAddr(descriptorPool));

		vkDestroyDescriptorPool(renderer->logicalDevice, descriptorPool, nullptr);
		descriptorPool = nullptr;

		logger.Info("TriangleScene::~TriangleScene DestroyDescriptorPool done");
	}

	if (pipeline)
	{
		logger.Info("TriangleScene::~TriangleScene DestroyPipeline " + logger.GetAddr(pipeline));

		vkDestroyPipeline(renderer->logicalDevice, pipeline, nullptr);
		pipeline = nullptr;

		logger.Info("TriangleScene::~TriangleScene DestroyPipeline done");
	}

	if (pipelineLayout)
	{
		logger.Info("TriangleScene::~TriangleScene DestroyPipelineLayout " + logger.GetAddr(pipelineLayout));

		vkDestroyPipelineLayout(renderer->logicalDevice, pipelineLayout, nullptr);
		pipelineLayout = nullptr;

		logger.Info("TriangleScene::~TriangleScene DestroyPipelineLayout done");
	}
	if (descriptorSetLayout)
	{
		logger.Info("TriangleScene::~TriangleScene DestroyDescriptorSetLayout " + logger.GetAddr(descriptorSetLayout));

		vkDestroyDescriptorSetLayout(renderer->logicalDevice, descriptorSetLayout, nullptr);
		descriptorSetLayout = nullptr;

		logger.Info("TriangleScene::~TriangleScene DestroyDescriptorSetLayout done");
	}

	if (uniformBufferMemory)
	{
		logger.Info("TriangleScene::~TriangleScene FreeMemory uniformBufferMemory " + logger.GetAddr(uniformBufferMemory));

		vkFreeMemory(renderer->logicalDevice, uniformBufferMemory, nullptr);
		uniformBufferMemory = nullptr;

		logger.Info("TriangleScene::~TriangleScene FreeMemory uniformBufferMemory done");
	}
	if (uniformBuffer)
	{
		logger.Info("TriangleScene::~TriangleScene DestroyBuffer uniformBuffer " + logger.GetAddr(uniformBuffer));

		vkDestroyBuffer(renderer->logicalDevice, uniformBuffer, nullptr);
		uniformBuffer = nullptr;

		logger.Info("TriangleScene::~TriangleScene DestroyBuffer uniformBuffer done");
	}

	if (vertexBufferMemory)
	{
		logger.Info("TriangleScene::~TriangleScene FreeMemory vertexBufferMemory " + logger.GetAddr(vertexBufferMemory));

		vkFreeMemory(renderer->logicalDevice, vertexBufferMemory, nullptr);
		vertexBufferMemory = nullptr;

		logger.Info("TriangleScene::~TriangleScene FreeMemory vertexBufferMemory done");
	}
	if (vertexBuffer)
	{
		logger.Info("TriangleScene::~TriangleScene DestroyBuffer vertexBuffer " + logger.GetAddr(vertexBuffer));

		vkDestroyBuffer(renderer->logicalDevice, vertexBuffer, nullptr);
		vertexBuffer = nullptr;

		logger.Info("TriangleScene::~TriangleScene DestroyBuffer vertexBuffer done");
	}
	if (indexBufferMemory)
	{
		logger.Info("TriangleScene::~TriangleScene FreeMemory indexBufferMemory " + logger.GetAddr(indexBufferMemory));

		vkFreeMemory(renderer->logicalDevice, indexBufferMemory, nullptr);
		indexBufferMemory = nullptr;

		logger.Info("TriangleScene::~TriangleScene FreeMemory indexBufferMemory done");
	}
	if (indexBuffer)
	{
		logger.Info("TriangleScene::~TriangleScene DestroyBuffer indexBuffer " + logger.GetAddr(indexBuffer));

		vkDestroyBuffer(renderer->logicalDevice, indexBuffer, nullptr);
		indexBuffer = nullptr;

		logger.Info("TriangleScene::~TriangleScene DestroyBuffer indexBuffer done");
	}
}

void TriangleScene::Initiate(void * renderer, ContentManager * contentManager)
{
	logger.Info("TriangleScene::Initiate ...");

	this->renderer = (RendererForward*)renderer;
	this->contentManager = contentManager;

	PrepareVericies();
	PrepareUniformBuffers();

	CreateDescriptorSetLayout();
	CreatePipelines();
	CreateDescriptorPool();
	CreateDescriptorSet();
	CreateCommandBuffers();

	hud.Initiate(renderer, contentManager);

	logger.Info("TriangleScene::Initiate done");
}

void TriangleScene::Update(double deltaTimer)
{
	hud.BeginTextUpdate();
	hud.AddText("Hello", 10, 10, HudManager::Left);
	hud.EndTextUpdate();

	UpdateUniformBuffers();
}

void TriangleScene::Render()
{
	hud.Render();
}

void TriangleScene::ResizeResources(uint32_t width, uint32_t height)
{
	logger.Info("TriangleScene::ResizeResources " + std::to_string(width) + "x" + std::to_string(height));

	CreateCommandBuffers();

	logger.Info("TriangleScene::ResizeResources done");
}

void TriangleScene::PrepareVericies()
{
	logger.Info("TriangleScene::PrepareVericies ...");

	struct Vertex
	{
		float position[3];
		float color[3];
	};

	// Setup vertices
	std::vector<Vertex> verticies =
	{
		{ { 1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
		{ { -1.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
		{ { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
	};
	uint32_t vertexBufferSize = static_cast<uint32_t>(verticies.size()) * sizeof(Vertex);

	// Setup indices
	std::vector<uint32_t> indices = { 0, 1, 2 };
	indexCount = static_cast<uint32_t>(indices.size());
	uint32_t indexBufferSize = indexCount * sizeof(uint32_t);

	renderer->StageBuffer(verticies.data(), vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, &vertexBuffer, &vertexBufferMemory);
	renderer->StageBuffer(indices.data(), indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, &indexBuffer, &indexBufferMemory);

	// Vertex input binding
	inputBinding.binding = 0;
	inputBinding.stride = sizeof(Vertex);
	inputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	// Inpute attribute binding describe shader attribute locations and memory layouts
	// These match the following shader layout (see triangle.vert):
	//	layout (location = 0) in vec3 inPos;
	//	layout (location = 1) in vec3 inColor;
	inputAttributes.resize(2);

	// Attribute location 0: Position
	inputAttributes[0].binding = 0;
	inputAttributes[0].location = 0;
	inputAttributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	inputAttributes[0].offset = offsetof(Vertex, position);

	// Attribute location 1: Color
	inputAttributes[1].binding = 0;
	inputAttributes[1].location = 1;
	inputAttributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	inputAttributes[1].offset = offsetof(Vertex, color);

	// Assign to the vertex input state used for pipeline creation
	inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	inputState.pNext = nullptr;
	inputState.flags = 0;
	inputState.vertexBindingDescriptionCount = 1;
	inputState.pVertexBindingDescriptions = &inputBinding;
	inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(inputAttributes.size());
	inputState.pVertexAttributeDescriptions = inputAttributes.data();

	logger.Info("TriangleScene::PrepareVericies done");
}

void TriangleScene::PrepareUniformBuffers()
{
	logger.Info("TriangleScene::PrepareUniformBuffers ...");

	// Prepare and initialize a uniform buffer block containing shader uniforms
	// Single uniforms like in OpenGL are no longer present in Vulkan. All Shader uniforms are passed via uniform buffer blocks

	// Vertex shader uniform buffer block
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.allocationSize = 0;
	allocInfo.memoryTypeIndex = 0;

	// This buffer will be used as a uniform buffer
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = sizeof(worldViewProjection);
	bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	// Create a new buffer
	if (vkCreateBuffer(renderer->logicalDevice, &bufferInfo, nullptr, &uniformBuffer) != VK_SUCCESS)
		throw "failed to create uniform buffer";
	
	logger.Info("TriangleScene::PrepareUniformBuffers CreateBuffer " + logger.GetAddr(uniformBuffer));

	// Get memory requirements including size, alignment and memory type 
	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(renderer->logicalDevice, uniformBuffer, &memReqs);

	allocInfo.allocationSize = memReqs.size;
	// Get the memory type index that supports host visibile memory access
	// Most implementations offer multiple memory types and selecting the correct one to allocate memory from is crucial
	// We also want the buffer to be host coherent so we don't have to flush (or sync after every update.
	// Note: This may affect performance so you might not want to do this in a real world application that updates buffers on a regular base
	allocInfo.memoryTypeIndex = renderer->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Allocate memory for the uniform buffer
	if (vkAllocateMemory(renderer->logicalDevice, &allocInfo, nullptr, &uniformBufferMemory) != VK_SUCCESS)
		throw "Failed to allocate uniform buffer memory";
	
	logger.Info("TriangleScene::PrepareUniformBuffers AllocateMemory " + logger.GetAddr(uniformBufferMemory));

	// Bind memory to buffer
	if (vkBindBufferMemory(renderer->logicalDevice, uniformBuffer, uniformBufferMemory, 0) != VK_SUCCESS)
		throw "Failed to bind uniform buffer memory";

	logger.Info("TriangleScene::PrepareUniformBuffers BindBufferMemory " + logger.GetAddr(uniformBufferMemory));

	// Store information in the uniform's descriptor that is used by the descriptor set
	uniformDescriptor.buffer = uniformBuffer;
	uniformDescriptor.offset = 0;
	uniformDescriptor.range = sizeof(worldViewProjection);

	logger.Info("TriangleScene::PrepareUniformBuffers done");

	UpdateUniformBuffers();
}

void TriangleScene::CreateDescriptorSetLayout()
{
	logger.Info("TriangleScene::CreateDescriptorSetLayout ...");

	// Setup layout of descriptors used in this example
	// Basically connects the different shader stages to descriptors for binding uniform buffers, image samplers, etc.
	// So every shader binding should map to one descriptor set layout binding

	// Binding 0: Uniform buffer (Vertex shader)
	VkDescriptorSetLayoutBinding layoutBinding = {};
	layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBinding.descriptorCount = 1;
	layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
	descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayout.pNext = nullptr;
	descriptorLayout.bindingCount = 1;
	descriptorLayout.pBindings = &layoutBinding;

	if (vkCreateDescriptorSetLayout(renderer->logicalDevice, &descriptorLayout, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		throw "Failed to create descriptor set layout";

	logger.Info("TriangleScene::CreateDescriptorSetLayout CreateDescriptorSetLayout " + logger.GetAddr(descriptorSetLayout));

	// Create the pipeline layout that is used to generate the rendering pipelines that are based on this descriptor set layout
	// In a more complex scenario you would have different pipeline layouts for different descriptor set layouts that could be reused
	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
	pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pPipelineLayoutCreateInfo.pNext = nullptr;
	pPipelineLayoutCreateInfo.setLayoutCount = 1;
	pPipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;

	if (vkCreatePipelineLayout(renderer->logicalDevice, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		throw "Failed to create pipeline layout";

	logger.Info("TriangleScene::CreateDescriptorSetLayout CreatePipelineLayout " + logger.GetAddr(pipelineLayout));
}

void TriangleScene::CreatePipelines()
{
	logger.Info("TriangleScene::CreatePipelines ...");

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
	rasterizationState.cullMode = VK_CULL_MODE_NONE;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	shaderStages.resize(2);

	// Vertex shader
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = contentManager->LoadShaderSPIRV(renderer->logicalDevice, "src/Shaders/triangle.vert.spv");
	shaderStages[0].pName = "main";

	// Fragment shader
	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = contentManager->LoadShaderSPIRV(renderer->logicalDevice, "src/Shaders/triangle.frag.spv");
	shaderStages[1].pName = "main";

	// Set pipeline shader stage info
	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	// The layout used for this pipeline (can be shared among multiple pipelines using the same layout)
	pipelineCreateInfo.layout = pipelineLayout;
	// Renderpass this pipeline is attached to
	pipelineCreateInfo.renderPass = renderer->renderPass;

	pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCreateInfo.pStages = shaderStages.data();

	// Assign the pipeline states to the pipeline creation info structure
	pipelineCreateInfo.pVertexInputState = &inputState;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.pDynamicState = &dynamicState;

	// Create rendering pipeline using the specified states
	if (vkCreateGraphicsPipelines(renderer->logicalDevice, renderer->pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS)
		throw "Failed to create graphics pipeline";
	
	for (auto shaderStage : shaderStages)
	{
		vkDestroyShaderModule(renderer->logicalDevice, shaderStage.module, nullptr);
		shaderStage.module = VK_NULL_HANDLE;
	}

	logger.Info("TriangleScene::CreatePipelines " + logger.GetAddr(pipeline));
}

void TriangleScene::CreateDescriptorPool()
{
	logger.Info("TriangleScene::CreateDescriptorPool ...");

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
	descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	descriptorPoolInfo.poolSizeCount = 1;
	descriptorPoolInfo.pPoolSizes = typeCounts;
	// Set the max. number of descriptor sets that can be requested from this pool (requesting beyond this limit will result in an error)
	descriptorPoolInfo.maxSets = 1;

	if (vkCreateDescriptorPool(renderer->logicalDevice, &descriptorPoolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		throw "Failed to create descriptor pool";

	logger.Info("TriangleScene::CreateDescriptorPool " + logger.GetAddr(descriptorPool));
}

void TriangleScene::CreateDescriptorSet()
{
	logger.Info("TriangleScene::CreateDescriptorSet ...");

	// Allocate a new descriptor set from the global descriptor pool
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayout;

	if (vkAllocateDescriptorSets(renderer->logicalDevice, &allocInfo, &descriptorSet) != VK_SUCCESS)
		throw "failed to allocate descriptor sets";

	logger.Info("TriangleScene::CreateDescriptorSet " + logger.GetAddr(descriptorSet));

	// Update the descriptor set determining the shader binding points
	// For every binding point used in a shader there needs to be one
	// descriptor set matching that binding point

	// Binding 0 : Uniform buffer
	VkWriteDescriptorSet writeDescriptorSet = {};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.dstSet = descriptorSet;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSet.pBufferInfo = &uniformDescriptor;
	// Binds this uniform buffer to binding point 0
	writeDescriptorSet.dstBinding = 0;

	vkUpdateDescriptorSets(renderer->logicalDevice, 1, &writeDescriptorSet, 0, nullptr);
}

void TriangleScene::CreateCommandBuffers()
{
	logger.Info("TriangleScene::CreateCommandBuffers ...");

	VkCommandBufferBeginInfo cmdBufInfo = {};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufInfo.pNext = nullptr;

	// Set clear values for all framebuffer attachments with loadOp set to clear
	// We use two attachments (color and depth) that are cleared at the start of the subpass and as such we need to set clear values for both
	VkClearValue clearValues[2];
	clearValues[0].color = { { 0.0f, 0.0f, 0.2f, 1.0f } };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = nullptr;
	renderPassBeginInfo.renderPass = renderer->renderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = renderer->width;
	renderPassBeginInfo.renderArea.extent.height = renderer->height;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;

	for (int32_t i = 0; i < renderer->renderCommandBuffers.size(); ++i)
	{
		// Set target frame buffer
		renderPassBeginInfo.framebuffer = renderer->frameBuffers[i];

		if (vkBeginCommandBuffer(renderer->renderCommandBuffers[i], &cmdBufInfo) != VK_SUCCESS)
			throw "Failed to begin render command buffer";

		// Start the first sub pass specified in our default render pass setup by the base class
		// This will clear the color and depth attachment
		vkCmdBeginRenderPass(renderer->renderCommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Update dynamic viewport state
		VkViewport viewport = {};
		viewport.height = (float)renderer->height;
		viewport.width = (float)renderer->width;
		viewport.minDepth = (float) 0.0f;
		viewport.maxDepth = (float) 1.0f;
		vkCmdSetViewport(renderer->renderCommandBuffers[i], 0, 1, &viewport);

		// Update dynamic scissor state
		VkRect2D scissor = {};
		scissor.extent.width = renderer->width;
		scissor.extent.height = renderer->height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(renderer->renderCommandBuffers[i], 0, 1, &scissor);

		// Bind descriptor sets describing shader binding points
		vkCmdBindDescriptorSets(renderer->renderCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

		// Bind the rendering pipeline
		// The pipeline (state object) contains all states of the rendering pipeline, binding it will set all the states specified at pipeline creation time
		vkCmdBindPipeline(renderer->renderCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		// Bind triangle vertex buffer (contains position and colors)
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(renderer->renderCommandBuffers[i], 0, 1, &vertexBuffer, offsets);

		// Bind triangle index buffer
		vkCmdBindIndexBuffer(renderer->renderCommandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		// Draw indexed triangle
		vkCmdDrawIndexed(renderer->renderCommandBuffers[i], indexCount, 1, 0, 0, 1);

		vkCmdEndRenderPass(renderer->renderCommandBuffers[i]);

		// Ending the render pass will add an implicit barrier transitioning the frame buffer color attachment to 
		// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR for presenting it to the windowing system

		if (vkEndCommandBuffer(renderer->renderCommandBuffers[i]) != VK_SUCCESS)
			throw "Failed to end command buffer";

		logger.Info("TriangleScene::CreateCommandBuffers " + logger.GetAddr(renderer->renderCommandBuffers[i]));
	}
}

void TriangleScene::UpdateUniformBuffers()
{
	// Update matrices
	worldViewProjection.projection = glm::perspective(glm::radians(60.0f), (float)renderer->width / (float)renderer->height, 0.1f, 256.0f);

	auto zoom = -2.5f;
	worldViewProjection.view = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, zoom));

	worldViewProjection.world = glm::mat4();
	worldViewProjection.world = glm::rotate(worldViewProjection.world, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	worldViewProjection.world = glm::rotate(worldViewProjection.world, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	worldViewProjection.world = glm::rotate(worldViewProjection.world, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	// Map uniform buffer and update it
	uint8_t *pData;
	if (vkMapMemory(renderer->logicalDevice, uniformBufferMemory, 0, sizeof(worldViewProjection), 0, (void **)&pData) != VK_SUCCESS)
		throw "Failed to map uniform buffer";

	memcpy(pData, &worldViewProjection, sizeof(worldViewProjection));

	// Unmap after data has been copied
	// Note: Since we requested a host coherent memory type for the uniform buffer, the write is instantly visible to the GPU
	vkUnmapMemory(renderer->logicalDevice, uniformBufferMemory);
}
