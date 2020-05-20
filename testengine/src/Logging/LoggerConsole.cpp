#include "LoggerConsole.h"

void LoggerConsole::Info(std::string message)
{
	auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

	PrintTimeStamp();

	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

	printf(message.c_str());
	printf("\n");
}

void LoggerConsole::Error(std::string message)
{
	auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_RED);

	PrintTimeStamp();

	printf(message.c_str());
	printf("\n");

	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

void LoggerConsole::PrintTimeStamp()
{
	auto now = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(now);

	tm t {};
	localtime_s(&t, &time);

	std::string message;
	message += "[" + std::to_string(t.tm_hour) + ":" + std::to_string(t.tm_min) + "." + std::to_string(t.tm_sec) + "] ";
	printf(message.c_str());

}
