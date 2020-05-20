#pragma once

#include <vector>
#include <array>

#include <vulkan\vulkan.h>

#include "DeviceManager.h"

class Renderer
{

public:

	VkQueue graphicsQueue;
	VkClearColorValue clearColor = { 0.0f, 0.0f, 0.2f, 1.0f };

	void Initialize(DeviceManager* deviceManager, VkSurfaceKHR surface, int width, int height);
	void BuildPipelines(std::vector<VkPipelineShaderStageCreateInfo> shaderStages);
	void BuildDescriptorSets(VkDescriptorBufferInfo* descriptorBufferInfo);
	void BuildDrawCommandBuffers(VkBuffer* vertices, VkBuffer* indices, uint32_t indexCount);

	void RenderFrame();

	void ResizeResources(std::vector<VkPipelineShaderStageCreateInfo> shaderStages, VkDescriptorBufferInfo* descriptorBufferInfo, VkBuffer* vertices, VkBuffer* indices, uint32_t indexCount);

	~Renderer();

protected:

	int width;
	int height;

	bool verticalSync = true;

	DeviceManager* deviceManager;

	VkSurfaceKHR surface;
	
	struct {

		VkSwapchainKHR swapchain = VK_NULL_HANDLE;
		VkFormat swapchainColorFormat;
		VkColorSpaceKHR swapchainColorSpace;
		uint32_t swapchainImageCount;
		uint32_t swapchainCurrentImage = 0;
		std::vector<VkImage> swapchainImages;
		std::vector<VkImageView> swapchainImageViews;

		std::vector<VkCommandBuffer> drawCommandBuffers;

		VkFormat depthStencilFormat;
		VkImage depthStencilImage;
		VkDeviceMemory depthStencilImageMemory;
		VkImageView depthStencilImageView;

		VkRenderPass renderPass;

		VkPipelineCache pipelineCache;
		
		std::vector<VkFramebuffer> frameBuffers;

		VkSemaphore semaphorePresent;
		VkSemaphore semaphoreRender;

		std::vector<VkFence> drawFences;

		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorPool descriptorPool;
		VkDescriptorSet descriptorSet;

		VkPipelineLayout pipelineLayout;
		VkVertexInputBindingDescription vertexInputBindingDescription;
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;
		VkPipelineVertexInputStateCreateInfo pipelineVertexInputState;
		VkPipeline pipeline;

	} resources;

	void SelectDepthFormat();

	void CreateGraphicsQueue();
	void CreateSwapChain();
	void CreateDrawCommandBuffers();
	void CreateDepthStencil();
	void CreateRenderPass();
	void CreatePipelineCache();
	void CreateDrawFrameBuffers();
	void CreateSemaphores();
	void CreateDescriptorSetLayout();
	void CreatePipelineLayout();
	void CreatePipelineVertexInputState();
	void CreateDescriptorPool();
	void CreateDescriptorSet();
};