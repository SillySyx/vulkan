#pragma once

#include "LoggerBase.h"

#include <stdio.h>
#include <Windows.h>
#include <chrono>

class LoggerConsole : public LoggerBase
{
public:

	virtual void Info(std::string message) override;
	virtual void Error(std::string message) override;

protected:

	void PrintTimeStamp();
};