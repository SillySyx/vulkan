#pragma once

#include <functional>

class RendererBase
{
public:

	uint32_t width = 1280;
	uint32_t height = 720;

	bool vsync = false;

	virtual void Initiate() = 0;
	virtual void Render(std::function<void()> fnc = 0) = 0;

	virtual void ResizeResources(uint32_t width, uint32_t height) = 0;

};