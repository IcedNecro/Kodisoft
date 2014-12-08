#include "stdafx.h"
#include "Logger.h"

#pragma comment(lib, "advapi32.lib")


const char* Logger::ERR_PROCESS_FATAL_ERROR = "FATAL ERROR code=%xi: Process %S crashed!";
const char* Logger::ERR_FAILED_WHILE_START = "ERROR code=%i: Process %S failed while starting";
const char* Logger::ERR_FAILED_WHILE_RESTART = "ERROR code=%i: Cannot restart process %S";
const char* Logger::ERR_WHILE_OPEN_PROCESS = "ERROR code=%i: Cannot open process with specified id=%i";

const char* Logger::WARN_ACCESS_DENIED = "WARN code=%i: Process is Locked - no access";
const char* Logger::WARN_DEBUGGER_ATTACH_ENABLED = "WARN code=%i: Process doesn\'t support debug";

const char* Logger::LOG_PROCESS_STARTED = "Process %S started successfully";
const char* Logger::LOG_PROCESS_RESUMED = "Process %S resumed";
const char* Logger::LOG_PROCESS_SUSPENDED = "Process %S suspended";
const char* Logger::LOG_PROCESS_STOPPED = "Process %S stopped";
const char* Logger::LOG_PROCESS_RESTARTED = "Process %S restarted";
const char* Logger::LOG_PROCESS_OPENNED = "Process %S was opened by pID %i";
const char* Logger::LOG_DEBUGGER_ATTACHED = "Debbuger attached to %S process with pID %i";

Logger::Logger(LPWSTR filename, ProcessManager * manager)
{

	HANDLE hEventSource = ::RegisterEventSource(NULL, _T("Kodisoft Test"));

	LPCWSTR* strings = new LPCWSTR[1];
	strings[0] = L"Proc1";
}

void Logger::log(DWORD eventID, ProcessManager * procManager)
{
	LPCSTR * _report_event = new LPCSTR[1];
	char* _str = new char[1000];
	cout << procManager->getProcessPath() <<" path"<< endl;
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
//	case 
	}

	*_report_event = LPCSTR(_str);


	::ReportEventA(
		hEventSource, EVENTLOG_SUCCESS,
		PROCESS_MANAGER_LOG, eventID, NULL, 1, 0, _report_event, NULL);
}

void Logger::error(DWORD errorID, ProcessManager * procManager, DWORD sysErrId)
{
	LPCSTR * _report_event = new LPCSTR[1];
	char* _str = new char[300];

	HANDLE hEventSource = ::RegisterEventSource(NULL, _T("Kodisoft Test"));

	switch (errorID)
	{
	case PROC_FAILED_WHILE_STARTED:
		sprintf_s(_str, 1000, Logger::ERR_FAILED_WHILE_START, sysErrId, procManager->getProcessPath());
		break;
	case PROC_ACCESS_DENIED:
		sprintf_s(_str, 1000, Logger::ERR_FAILED_WHILE_START, sysErrId, procManager->getProcessPath());
		break;
	case PROC_FATAL_ERROR:
		sprintf_s(_str, 1000, Logger::ERR_FAILED_WHILE_START, sysErrId, procManager->getProcessPath());
		break;
	}

	*_report_event = LPCSTR(_str);

	::ReportEventA(
		hEventSource, EVENTLOG_ERROR_TYPE,
		PROCESS_MANAGER_ERRORS, errorID, NULL, 1, 0, _report_event, NULL);
}

Logger::~Logger()
{
}
