#pragma once

#include "ContentManager.h"
#include "../Renderers/RendererForward.h"
#include "../Logging/LoggerConsole.h"

#include "stb_font_consolas_24_latin1.inl"
#define STB_FONT_NAME stb_font_consolas_24_latin1
#define STB_FONT_WIDTH STB_FONT_consolas_24_latin1_BITMAP_WIDTH
#define STB_FONT_HEIGHT STB_FONT_consolas_24_latin1_BITMAP_HEIGHT 
#define STB_FIRST_CHAR STB_FONT_consolas_24_latin1_FIRST_CHAR
#define STB_NUM_CHARS STB_FONT_consolas_24_latin1_NUM_CHARS

#define MAX_CHAR_COUNT 2048

class HudManager
{
public:

	enum TextAlign { Left, Center, Right };

	~HudManager();

	void Initiate(void * renderer, void * contentManager);

	void Render();

	void BeginTextUpdate();
	void EndTextUpdate();

	void AddText(std::string text, float x, float y, TextAlign align);

protected:

	LoggerConsole logger;

	RendererForward * renderer = VK_NULL_HANDLE;
	ContentManager * contentManager = VK_NULL_HANDLE;

	std::vector<VkCommandBuffer> cmdBuffers;
	VkPipelineShaderStageCreateInfo shaderStages[2];

	VkCommandPool commandPool = VK_NULL_HANDLE;

	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
	void * mappedVertexBuffer = VK_NULL_HANDLE;

	stb_fontchar stbFontData[STB_NUM_CHARS];
	VkImage fontImage = VK_NULL_HANDLE;
	VkDeviceMemory fontImageMemory = VK_NULL_HANDLE;
	VkImageView fontImageView = VK_NULL_HANDLE;

	VkSampler sampler = VK_NULL_HANDLE;

	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkPipelineCache pipelineCache = VK_NULL_HANDLE;

	VkFence fence = VK_NULL_HANDLE;
	VkRenderPass renderPass = VK_NULL_HANDLE;
	VkPipeline pipeline = VK_NULL_HANDLE;

	// Used during text updates
	glm::vec4 * mappedTextData = VK_NULL_HANDLE;
	uint32_t numLetters;
	float scale = 1.0f;

	void InitCommandPool();
	void DestroyCommandPool();

	void InitVertexBuffer();
	void DestroyVertexBuffer();

	void InitFontTexture();
	void DestroyFontTexture();

	void InitFontSampler();
	void DestroyFontSampler();

	void InitDescriptorPool();
	void DestroyDescriptorPool();

	void InitDescriptorSetLayout();
	void DestroyDescriptorSetLayout();

	void InitPipelineLayout();
	void DestroyPipelineLayout();

	void InitPipelineCache();
	void DestroyPipelineCache();

	void InitFences();
	void DestroyFences();

	void InitRenderPass();
	void DestroyRenderPass();

	void InitPipeline();
	void DestroyPipeline();

	void UpdateCommandBuffers();
};