#include "Process.h"
#include "Logger.h"

#pragma comment(lib, "advapi32.lib")


const char* Logger::ERR_FAILED_WHILE_START = "ERROR: Process %i failed while starting";
const char* Logger::ERR_FAILED_WHILE_RESTART = "ERROR: Cannot restart process %S";
const char* Logger::ERR_WHILE_OPEN_PROCESS = "ERROR: Cannot open process with specified id = %i";

const char* Logger::WARN_ACCESS_DENIED = "WARN code=%i: Process %i is Locked - no access";
const char* Logger::WARN_DEBUGGER_ATTACH_DISABLED = "WARN code=%i: Process %i doesn\'t support debug";
const char* Logger::WARN_PROCESS_EXIT = "WARN code=%i: Process crashed or exited";

const char* Logger::LOG_PROCESS_STARTED = "Process %S started successfully";
const char* Logger::LOG_PROCESS_RESUMED = "Process %S resumed";
const char* Logger::LOG_PROCESS_SUSPENDED = "Process %S suspended";
const char* Logger::LOG_PROCESS_STOPPED = "Process %S stopped";
const char* Logger::LOG_PROCESS_RESTARTED = "Process %S restarted";
const char* Logger::LOG_PROCESS_OPENNED = "Process %S was opened by pID %i";
const char* Logger::LOG_DEBUGGER_ATTACHED = "Debbuger attached to %S process with pID %i";

void Logger::log(DWORD eventID, ProcessManager * procManager)
{
	LPCSTR * _report_event = new LPCSTR[1];
	char* _str = new char[1000];
	HANDLE hEventSource = ::RegisterEventSource(NULL, _T("Kodisoft Test"));

	switch (eventID)
	{
	case PROC_SUCCESSFULLY_STARTED:
		sprintf_s(_str, 1000, Logger::LOG_PROCESS_STARTED, procManager->getProcessPath());
		break;
	case PROC_SUSPEND_EVENT:
		sprintf_s(_str, 1000, Logger::LOG_PROCESS_SUSPENDED, procManager->getProcessPath());
		break;
	case PROC_RESUME_EVENT:
		sprintf_s(_str, 1000, Logger::LOG_PROCESS_RESUMED, procManager->getProcessPath());
		break;
	case PROC_STOP_EVENT:
		sprintf_s(_str, 1000, Logger::LOG_PROCESS_STOPPED, procManager->getProcessPath());
		break;
	case PROC_RESTART_EVENT:
		sprintf_s(_str, 1000, Logger::LOG_PROCESS_RESTARTED, procManager->getProcessPath());
		break;
	case PROC_OPEN_EVENT:
		sprintf_s(_str, 1000, Logger::LOG_PROCESS_OPENNED, procManager->getProcessPath(), procManager->pId);
		break;
	case DEB_ATTACH_SUCCESS:
		sprintf_s(_str, 1000, Logger::LOG_DEBUGGER_ATTACHED, procManager->getProcessPath(), procManager->pId);
		break;
	}

	*_report_event = LPCSTR(_str);
	
	::ReportEventA(
		hEventSource, EVENTLOG_SUCCESS,
		PROCESS_MANAGER_LOG, eventID, NULL, 1, 0, _report_event, NULL);
}

void Logger::warning(DWORD eventID, ProcessManager * procManager, DWORD sysErrCode)
{
	LPCSTR * _report_event = new LPCSTR[1];
	char* _str = new char[1000];
	HANDLE hEventSource = ::RegisterEventSource(NULL, _T("Kodisoft Test"));

	switch (eventID)
	{
	case PROC_ACCESS_DENIED:
		sprintf_s(_str, 1000, Logger::WARN_ACCESS_DENIED, sysErrCode, procManager->pId);
		break;
	case DEB_ATTACH_FAIL:
		sprintf_s(_str, 1000, Logger::WARN_DEBUGGER_ATTACH_DISABLED, sysErrCode, procManager->pId);
		break;
	case PROC_FATAL_ERROR:
		sprintf_s(_str, 1000, Logger::WARN_PROCESS_EXIT, procManager->getProcessPath());
		break;
	}

	*_report_event = LPCSTR(_str);
	
	::ReportEventA(	hEventSource, EVENTLOG_WARNING_TYPE,PROCESS_MANAGER_LOG, eventID, NULL, 1, 0, _report_event, NULL);
}

void Logger::error(DWORD errorID, ProcessManager * procManager, DWORD sysErrId)
{
	LPCSTR * _report_event = new LPCSTR[1];
	char* _str = new char[1000];

	HANDLE hEventSource = ::RegisterEventSource(NULL, _T("Kodisoft Test"));

	switch (errorID)
	{
	case PROC_FAILED_WHILE_STARTED:	
		sprintf_s(_str, 1000, Logger::ERR_FAILED_WHILE_START, procManager->pId);
		break;
	case PROC_FAILED_WHILE_OPEN:
		sprintf_s(_str, 1000, Logger::ERR_WHILE_OPEN_PROCESS, procManager->pId);
		break;
	}

	*_report_event = LPCSTR(_str);

	::ReportEventA(	hEventSource, EVENTLOG_ERROR_TYPE,PROCESS_MANAGER_ERRORS, errorID, NULL, 1, 0, _report_event, NULL);
}

Logger::~Logger()
{
}
