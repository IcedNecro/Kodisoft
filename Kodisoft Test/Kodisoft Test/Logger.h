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
	static const char* WARN_DEBUGGER_ATTACH_DISABLED;
	static const char* WARN_PROCESS_EXIT;

	static const char* LOG_PROCESS_STARTED;
	static const char* LOG_PROCESS_SUSPENDED;
	static const char* LOG_PROCESS_RESUMED;
	static const char* LOG_PROCESS_STOPPED;
	static const char* LOG_PROCESS_RESTARTED;
	static const char* LOG_DEBUGGER_ATTACHED;
	static const char* LOG_PROCESS_OPENNED;
public:

	/**
	 */
	static VOID warning(DWORD, ProcessManager*, DWORD);
	/**
	 * Static method to send error message to Event Log
	 * @arg eventID - id of event (specific ID from event_cat.h
	 * @arg procManager - pointer to ProcessManager object. As Logger is declared
	 * as friendly class to ProcessManager, Logger can access private fields of
	 * ProcessManager
	 * @arg sysErrId - error-code, recieved from GetLastError() function
	 */
	static VOID error(DWORD, ProcessManager*, DWORD);

	/**
	* Static method to send usual message to Event Log
	* @arg eventID - id of event (specific ID from event_cat.h
	* @arg procManager - pointer to ProcessManager object. As Logger is declared
	* as friendly class to ProcessManager, Logger can access private fields of
	* ProcessManager
	*/
	static VOID log(DWORD, ProcessManager*);
	~Logger();
};

