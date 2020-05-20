#pragma once

#include <string>

struct LoggerCreateInfo {
	const char * pName;
};

class BaseLogger
{
public:

	virtual void Info(std::string message) = 0;
	virtual void Error(std::string message) = 0;
};