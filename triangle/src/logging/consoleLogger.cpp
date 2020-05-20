#include "consoleLogger.h"

ConsoleLogger * ConsoleLogger::Create(LoggerCreateInfo * pCreateInfo)
{
	auto logger = new ConsoleLogger;

	logger->Initialize(pCreateInfo->pName);

	return logger;
}

ConsoleLogger::~ConsoleLogger()
{
	fflush(stdout);
}

void ConsoleLogger::Info(std::string message)
{
	auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

	PrintTimeStamp();

	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

	printf(message.c_str());
	printf("\n");
}

void ConsoleLogger::Error(std::string message)
{
	auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_RED);

	PrintTimeStamp();

	printf(message.c_str());
	printf("\n");

	SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

void ConsoleLogger::Initialize(const char * pName)
{
	auto title = std::string(pName) + std::string(" - debug console");

	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	FILE *stream;
	freopen_s(&stream, "CONOUT$", "w+", stdout);
	SetConsoleTitle(TEXT(title.c_str()));
}

void ConsoleLogger::PrintTimeStamp()
{
	auto now = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(now);

	tm t{};
	localtime_s(&t, &time);

	std::string message;
	message += "[" + std::to_string(t.tm_hour) + ":" + std::to_string(t.tm_min) + "." + std::to_string(t.tm_sec) + "] ";
	printf(message.c_str());
}
