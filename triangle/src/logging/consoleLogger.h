#pragma once

#include "baseLogger.h"

#include <stdio.h>
#include <Windows.h>
#include <chrono>

class ConsoleLogger : public BaseLogger
{
public:
	
	virtual void Info(std::string message) override;
	virtual void Error(std::string message) override;

	void Initialize(const char * pName);

	~ConsoleLogger();

	static ConsoleLogger * Create(LoggerCreateInfo* pCreateInfo);

protected:

	void PrintTimeStamp();
};