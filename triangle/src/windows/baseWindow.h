#pragma once

#include "..\logging\baseLogger.h"

class BaseWindow
{
public:
	BaseLogger * pLogger;

	bool shutdown = false;
	bool fullscreen = false;
	bool resizing = false;

	unsigned int width;
	unsigned int height;
	
	virtual void StartProcessingMessages() = 0;

	virtual void * GetHandle() = 0;
};