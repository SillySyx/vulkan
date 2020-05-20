#pragma once
#include <sstream>
#include <string>

class LoggerBase
{
public:

	virtual void Info(std::string message) = 0;
	virtual void Error(std::string message) = 0;

	virtual std::string GetAddr(void* ptr);
};