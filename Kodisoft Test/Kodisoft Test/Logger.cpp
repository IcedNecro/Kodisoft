#include "stdafx.h"
#include "Logger.h"

#pragma comment(lib, "advapi32.lib")


const char* Logger::ERR_PROCESS_FATAL_ERROR = "FATAL ERROR code=%xi: Process %S crashed!";
const char* Logger::ERR_FAILED_WHILE_START = "ERROR code=%xi: Process %S failed while starting";
const char* Logger::LOG_PROCESS_STARTED = "Process %S started successfully";
const char* Logger::LOG_PROCESS_RESUMED = "Process %S resumed";
const char* Logger::LOG_PROCESS_SUSPENDED = "Process %S suspended";
const char* Logger::LOG_PROCESS_STOPPED = "Procees %S stopped";


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

	HANDLE hEventSource = ::RegisterEventSource(NULL, _T("Kodisoft Test"));

	switch (eventID)
	{
	case PROC_SUCCESSFULLY_STARTED:
		sprintf_s(_str, 1000, Logger::LOG_PROCESS_STARTED, procManager->path);
	//	cout << _report_event[0] << endl;
		break;
	case PROC_SUSPEND_EVENT:
		sprintf_s(_str, 1000, Logger::LOG_PROCESS_SUSPENDED, procManager->path);
		break;
	case PROC_RESUME_EVENT:
		sprintf_s(_str, 1000, Logger::LOG_PROCESS_RESUMED, procManager->path);
		break;
	case PROC_STOP_EVENT:
		sprintf_s(_str, 1000, Logger::LOG_PROCESS_STOPPED, procManager->path);
		break;
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
		sprintf_s(_str, 1000, Logger::ERR_FAILED_WHILE_START, sysErrId, procManager->path);
		break;
	case PROC_ACCESS_DENIED:
		sprintf_s(_str, 1000, Logger::ERR_FAILED_WHILE_START, sysErrId, procManager->path);
		break;
	case PROC_FATAL_ERROR:
		sprintf_s(_str, 1000, Logger::ERR_FAILED_WHILE_START, sysErrId, procManager->path);
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
