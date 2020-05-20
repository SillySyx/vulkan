#include "LoggerBase.h"

std::string LoggerBase::GetAddr(void * ptr)
{
	std::ostringstream addr;
	addr << &ptr;

	return addr.str();
}
