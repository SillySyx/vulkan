#pragma once
#include "../Engine/BaseApplication.h"

class RendererApp : public BaseApplication
{

public:

	virtual void Load() override;
	virtual void UnLoad() override;

	virtual void Update(__int64 time) override;
	virtual void DrawFrame() override;

	virtual void WindowSizeChanged() override;

	~RendererApp();

protected:

	Renderer renderer;
	MeshManager meshManager;
	SceneManager sceneManager;

	glm::vec3 rotation = glm::vec3();
	glm::vec3 position = glm::vec3(0.0f, 0.0f, -2.5f);
	glm::vec3 modelPosition = glm::vec3(0.f, 0.f, 0.f);

	// Vertex buffer and attributes
	struct
	{
		VkDeviceMemory memory;
		VkBuffer buffer;
	} vertices;

	// Index buffer
	struct
	{
		VkDeviceMemory memory;
		VkBuffer buffer;
		uint32_t count;
	} indices;

	// Uniform buffer block object
	struct
	{
		VkDeviceMemory memory;
		VkBuffer buffer;
		VkDescriptorBufferInfo descriptor;
	}  uniformBufferVS;

	// For simplicity we use the same uniform block layout as in the shader:
	//
	//	layout(set = 0, binding = 0) uniform UBO
	//	{
	//		mat4 projectionMatrix;
	//		mat4 modelMatrix;
	//		mat4 viewMatrix;
	//	} ubo;
	//
	// This way we can just memcopy the ubo data to the ubo
	// Note: You should use data types that align with the GPU in order to avoid manual padding (vec4, mat4)
	struct {
		glm::mat4 projectionMatrix;
		glm::mat4 modelMatrix;
		glm::mat4 viewMatrix;
	} uboVS;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	void LoadVertexAndIndexBuffers();
	void LoadShaders();

	void CreateUniformBuffers();
	void UpdateUniformBuffers();

};