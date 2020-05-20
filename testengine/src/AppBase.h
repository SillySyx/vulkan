#pragma once

class AppBase
{
public:
	bool fullscreen = false;

	const char * name = "VulkanTest";

	virtual void Start() = 0;
	virtual void HandleError(const char * message) = 0;

	virtual void WindowResized() = 0;
};