#include "ProcessManager.h"
#pragma once

class Logger
{
private:

	static const char* ERR_PROCESS_FATAL_ERROR;
	static const char* ERR_FAILED_WHILE_START;
	static const char* LOG_PROCESS_STARTED;
	static const char* LOG_PROCESS_SUSPENDED;
	static const char* LOG_PROCESS_RESUMED;
	static const char* LOG_PROCESS_STOPPED;
	ProcessManager * pProgressManager;
public:
	Logger(LPWSTR, ProcessManager *);
	//	void operator<<(LPSTR);
	static VOID error(DWORD, ProcessManager*, DWORD);
	static VOID log(DWORD, ProcessManager*);
	~Logger();
};

