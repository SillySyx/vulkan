#pragma once

#include <vector>
#include <memory>

#include "../Renderers/RendererForward.h"
#include "../Vulkan/VulkanWrapper.h"
#include "../Base/ContentManager.h"
#include "../Base/HudManager.h"
#include "../Logging/LoggerConsole.h"

class TriangleScene
{
public: 

	~TriangleScene();

	void Initiate(void * renderer, ContentManager * contentManager);

	void Update(double deltaTimer);

	void Render();

	void ResizeResources(uint32_t width, uint32_t height);

protected:

	LoggerConsole logger;
	RendererForward * renderer = VK_NULL_HANDLE;
	ContentManager * contentManager = VK_NULL_HANDLE;

	HudManager hud;

	VkDeviceMemory vertexBufferMemory;
	VkBuffer vertexBuffer;

	VkDeviceMemory indexBufferMemory;
	VkBuffer indexBuffer;
	uint32_t indexCount;

	VkPipelineVertexInputStateCreateInfo inputState;
	VkVertexInputBindingDescription inputBinding;
	std::vector<VkVertexInputAttributeDescription> inputAttributes;

	VkDeviceMemory uniformBufferMemory;
	VkBuffer uniformBuffer;
	VkDescriptorBufferInfo uniformDescriptor;

	VkPipelineLayout pipelineLayout;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipeline pipeline;
	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptorSet;

	glm::vec3 rotation = glm::vec3();

	struct
	{
		glm::mat4 world;
		glm::mat4 view;
		glm::mat4 projection;
	} worldViewProjection;

	void PrepareVericies();
	void PrepareUniformBuffers();
	void CreateDescriptorSetLayout();
	void CreatePipelines();
	void CreateDescriptorPool();
	void CreateDescriptorSet();
	void CreateCommandBuffers();

	void UpdateUniformBuffers();

};