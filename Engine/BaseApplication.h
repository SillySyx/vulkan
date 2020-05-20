#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdexcept>
#include <iostream>
#include <memory>
#include <chrono>
#include <string>
#include <sstream>

#include "WindowManager.h"
#include "DeviceManager.h"
#include "ShaderManager.h"
#include "MeshManager.h"
#include "SceneManager.h"
#include "Renderer.h"

class BaseApplication
{

public:

	bool enableValidation = false;

	virtual void Initialize(char* title, int width, int height);
	void Run();

	virtual void Load() = 0;
	virtual void UnLoad() = 0;

	virtual void Update(__int64 time) = 0;
	virtual void DrawFrame() = 0;

	virtual void WindowSizeChanged() = 0;

	~BaseApplication();

protected:

	WindowManager windowManager;
	DeviceManager deviceManager;
	ShaderManager shaderManager;

	bool shouldShutDown = false;
	int width;
	int height;

	static void WindowResized(GLFWwindow* window, int width, int height);
};