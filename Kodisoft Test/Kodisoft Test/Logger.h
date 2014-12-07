#include "ProcessManager.h"
#pragma once
/**
 * Logger class defined for logging ProcessManager events. It use Windows
 * event logging for making logs about all actions an errors caused by
 * ProcessManager
 * Logger uses event_cat.h constants that were generated and compiled by mc tool
 * from event_cat.mc to identify massage type and id. Also it contains a set of 
 * string message templates that are used to send logs to Event Log
 */
class Logger
{
private:

	static const char* ERR_PROCESS_FATAL_ERROR;
	static const char* ERR_FAILED_WHILE_START;
	static const char* ERR_FAILED_WHILE_RESTART;
	static const char* ERR_WHILE_OPEN_PROCESS;
	
	static const char* WARN_ACCESS_DENIED;
	static const char* WARN_DEBUGGER_ATTACH_ENABLED;

	static const char* LOG_PROCESS_STARTED;
	static const char* LOG_PROCESS_SUSPENDED;
	static const char* LOG_PROCESS_RESUMED;
	static const char* LOG_PROCESS_STOPPED;
	static const char* LOG_PROCESS_RESTARTED;
	static const char* LOG_DEBUGGER_ATTACHED;
	static const char* LOG_PROCESS_OPENNED;
	static const char* LOG_PROCESS_RESTARTED;
	ProcessManager * pProgressManager;
public:
	Logger(LPWSTR, ProcessManager *);
	//	void operator<<(LPSTR);
	static VOID error(DWORD, ProcessManager*, DWORD);
	static VOID log(DWORD, ProcessManager*);
	~Logger();
};

