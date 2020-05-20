#pragma once

#include "RendererVulkanBase.h"
#include <array>

class RendererForward : public RendererVulkanBase
{
public:

	uint32_t currentCommandBuffer = 0;
	std::vector<VkCommandBuffer> renderCommandBuffers;

	VkSwapchainKHR swapChain = VK_NULL_HANDLE;
	uint32_t swapChainImageCount;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;

	VkImage depthStencilImage = VK_NULL_HANDLE;
	VkImageView depthStencilImageView = VK_NULL_HANDLE;
	VkDeviceMemory depthStencilMemory = VK_NULL_HANDLE;

	VkRenderPass renderPass = VK_NULL_HANDLE;
	VkPipelineCache pipelineCache = VK_NULL_HANDLE;

	std::vector<VkFramebuffer> frameBuffers;

	std::vector<VkFence> waitFences;

	~RendererForward();

	virtual void Initiate() override;
	virtual void Render(std::function<void()> fnc) override;
	
	virtual void ResizeResources(uint32_t width, uint32_t height) override;

protected:

	void CreateCommandPool();
	void DestroyCommandPool();

	void CreateSwapChain();
	void DestroySwapChain();

	void CreateRenderCommandBuffers();
	void DestoryRenderCommandBuffers();

	void CreateDepthStencil();
	void DestoryDepthStencil();

	void CreateRenderPass();
	void DestoryRenderPass();

	void CreatePipelineCache();
	void DestoryPipelineCache();

	void CreateFrameBuffers();
	void DestroyFrameBuffers();

	void CreateFences();
	void DestroyFences();

};