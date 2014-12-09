#include "stdafx.h"
#include "ProcessManager.h"
#include "Logger.h"
#include "command_line_executor.h"

const DWORD ProcessManager::EXIT_CODE = 0x0000666;

char * ProcessManager::getTime()
{
	const char * dateTemplate = "[%2i-%2i-%2i %2i:%2i:%2i]";
	SYSTEMTIME st;
	GetSystemTime(&st);
	char * _str = new char[200];

	sprintf_s(_str, 200, dateTemplate, st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond);
	return _str;
}

bool ProcessManager::pauseProcess()
{
	// As we know, processes almost has a few threads. So to pause whole process
	// we should suspend all threads that are running with process. We use System
	// Snapshot to recieve a list of threads of process

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, this->pId);
	if (hProcess != NULL)
	{
		HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

		THREADENTRY32 threadEntry;
		threadEntry.dwSize = sizeof(THREADENTRY32);

		// getting first thread in list 
		Thread32First(hThreadSnapshot, &threadEntry);

		DWORD val;
		BOOL allThreadsRunning = true;

		// Loop where we get's every thread suspended
		do
		{
			if (threadEntry.th32OwnerProcessID == this->pId)
			{
				// Open thread by specific id
				HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE,
					threadEntry.th32ThreadID);

				// if thread was suspended early, SuspendThread() will return
				// a value that shows how much SuspendThread() was called for thread before
				// without resuming it by ResumeThread(). So, if we called SuspendThread() 3
				// times, we need call ResumeThread() 3 times to resume thread. This thing
				// makes that it's unable to call SuspendThread() more then 1 time without 
				// ResummeThread() being called
				val = SuspendThread(hThread);
				if (val != 0)
					ResumeThread(hThread);
				else
					this->isSuspended = true;
				CloseHandle(hThread);
			}
		} while (Thread32Next(hThreadSnapshot, &threadEntry));

		CloseHandle(hThreadSnapshot);
		this->onPause(this);
		if (val == 0)
			Logger::log(PROC_SUSPEND_EVENT, this);
		cout << ProcessManager::getTime() << " process " << this->pId << " paused " << endl;

		return this->isSuspended;

	}
	else 
		Logger::warning(PROC_ACCESS_DENIED, this, GetLastError());
}
bool ProcessManager::resumeProcess()
{
	// Here we do the same thing as in pauseProcess(), with only difference that
	// we calls ResumeThread() for each thread in process

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, this->pId);
	if (hProcess != NULL)
	{
		HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

		THREADENTRY32 threadEntry;
		threadEntry.dwSize = sizeof(THREADENTRY32);

		Thread32First(hThreadSnapshot, &threadEntry);

		BOOL allThreadsSuspended = true;

		DWORD val;
		do
		{
			if (threadEntry.th32OwnerProcessID == this->pId)
			{
				HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE,
					threadEntry.th32ThreadID);

				val = ResumeThread(hThread);

				CloseHandle(hThread);
			}
		} while (Thread32Next(hThreadSnapshot, &threadEntry));

		CloseHandle(hThreadSnapshot);

		// here we checks if there are more than 1 time SuspendThread() being called
		// I don't know why I put it here, because in pauseProcess() we have already made
		// SuspendThread() to be called exactly 1 time, but let it be :)
		if (val <= 1)
			this->isSuspended = false;
		else this->isSuspended = true;

		Logger::log(PROC_RESUME_EVENT, this);

		this->onResume(this);
		cout << ProcessManager::getTime() << " process " << this->pId << " resumed " << endl;
		return this->isSuspended;
	}
	else
		Logger::warning(PROC_ACCESS_DENIED, this, GetLastError());
}

bool ProcessManager::stopProcess()
{
	// Here we terminates process with exit code 0, that shows that process was terminated
	// normally

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, this->pId);
	
	if (!TerminateProcess(hProcess, ProcessManager::EXIT_CODE))
	{
		cout << ProcessManager::getTime() << " ERROR: unable to stop process " << this->pId<<" code "<< GetLastError() << endl;
		return false;
	}
	else
	{
		cout << ProcessManager::getTime() << " process " << this->pId << " stopped" << endl; this->onStop(this);
		Logger::log(PROC_STOP_EVENT, this);
		this->onStop(this);
		return true;
	}
}

void ProcessManager::setOnProcessStartListener(void(*func)(ProcessManager *))
{
	this->onStart = func;
}

void ProcessManager::setOnProcessStopListener(void(*func)(ProcessManager *))
{
	this->onStop = func;
}

void ProcessManager::setOnProcessPauseListener(void(*func)(ProcessManager *))
{
	this->onPause = func;
}

void ProcessManager::setOnProcessResumeListener(void(*func)(ProcessManager *))
{
	this->onResume = func;
}

bool ProcessManager::restartProcess()
{
	STARTUPINFO cif;
	PROCESS_INFORMATION pi;
	
	// if debug thread was created before - we should detach it and delete
	if (t!=NULL )
	{
		t->detach();
		delete t;
	}
	//if process isn't stopped yet - it would be surely stopped
	//this->stopProcess();
	ZeroMemory(&cif, sizeof(STARTUPINFO));
	this->isSuspended = false;
	if (!CreateProcess(NULL, LPWSTR(this->path.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &cif, &pi))
	{
		Logger::error(PROC_FAILED_WHILE_STARTED, this, GetLastError());
		cout << ProcessManager::getTime() << " ERROR: process " << this->pId << " when restarted - code "<< GetLastError() << endl;
		return false;
	}
	else
	{
		// restart is successfull
		this->pId = DWORD(pi.dwProcessId);
		cout << ProcessManager::getTime() << " process " << this->pId << " restarted" << endl; 
		this->onStop(this);

		t = new thread(&ProcessManager::debugLoop, this);

		Logger::log(PROC_RESTART_EVENT, this);
		this->onStart(this);
		return true;
	}
}

bool ProcessManager::startProcess()
{
	SYSTEMTIME st;
	GetSystemTime(&st);

	STARTUPINFO cif;
	PROCESS_INFORMATION pi;
	
	ZeroMemory(&cif, sizeof(STARTUPINFO));
	this->isSuspended = false;
	
	// we do this to know if there isn't already launched process with such id
	// It makes to prevent start of already opened process (via Id)
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, this->pId);

	if (!CreateProcess(NULL, LPWSTR(this->path.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &cif, &pi) && hProcess==NULL)
	{
		Logger::error(PROC_FAILED_WHILE_STARTED, this, GetLastError());
		cout << ProcessManager::getTime() << " process start failed, or process is already running" << endl; 
		return false;
	}
	else
	{
		
		this->pId = DWORD(pi.dwProcessId);
		cout << ProcessManager::getTime() << " process " << this->pId << " started successfully " << endl;

		t = new thread(&ProcessManager::debugLoop, this);
		
		Logger::log(PROC_SUCCESSFULLY_STARTED, this);
		this->onStart(this);
		return true;
	}
}

ProcessManager::ProcessManager(LPWSTR _cmd_prompt_path, LPWSTR _args)
{
	// concatenation of comand line path and arguments to pass it to CreateProcess()
	this->path = wstring(_cmd_prompt_path) + wstring(L" ") + wstring(_args);

	// Initialization of default callback functions

	this->onStop = [](ProcessManager * pm)
	{
	};

	this->onResume = [](ProcessManager * pm)
	{
	};

	this->onStart = [](ProcessManager * pm)
	{
	};

	this->onPause = [](ProcessManager * pm)
	{
	}; 
}

ProcessManager::ProcessManager(DWORD pId)
{ 
	// Open process via pID

	this->pId = pId;

	HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, false, pId);

	if (handle == NULL)
	{
		DWORD err = GetLastError();

		if (err == ERROR_ACCESS_DENIED)
		{
			cout << getTime() << " unable to open " << pId << ": access denied" << endl;
			Logger::warning(PROC_ACCESS_DENIED, this, err);
		}
		else
		{
			cout << getTime() << " unable to open " << pId << endl;
			Logger::error(PROC_FAILED_WHILE_OPEN, this, err);
		}
	}
	else
	{
		PWSTR buf;
		// getting command line of running process
		get_cmd_line(pId, buf);
		this->path = buf;
		this->isSuspended = false;

		// default callbacks

		this->onStop = [](ProcessManager * pm)
		{
		};

		this->onResume = [](ProcessManager * pm)
		{
		};

		this->onStart = [](ProcessManager * pm)
		{
		};

		this->onPause = [](ProcessManager * pm)
		{
		};
		Logger::log(PROC_OPEN_EVENT, this);

		cout << getTime() << " process opened " << pId << endl;
		t = new thread(ProcessManager::debugLoop, this);
	}
}

HANDLE& ProcessManager::getProcessHandle()
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, pId);

	return hProcess;
}

void ProcessManager::debugLoop(ProcessManager *manager)
{
	// The worst function I've ever wrote
	// This is Debug Loop for our process. I don't know why some apps can be debbuged, but other not
	// Looks like it's a properly way to handle application crash, but even it cannot handle all processes

	if (DebugActiveProcess(manager->pId))
	{
		// debug is enabled

		Logger::log(DEB_ATTACH_SUCCESS, manager);
		cout << ProcessManager::getTime() <<" debuger attached to process "<< manager->pId << endl;
		// debug loop
		while (1)
		{
			DWORD exit_code;
			DEBUG_EVENT dEvent;
			
			// getting debug event
			if (WaitForDebugEvent(&dEvent,INFINITE))
			{
				GetExitCodeProcess(manager->getProcessHandle(), &exit_code);
 
				// here I've got a very strange ocassion
				// I've thought to check EXCEPTION_DEBUG_EVENT - if we have an exception thrown (app crash)
				// - we restarts process. I read that when we starts debug with process startup, system puts breakpoint, which
				// I need to skip. But also debugger raises ACCESS_VIOLATION exception at the start of process,
				// that automaticaly crashes process and makes it unable to launch again. What is strange - when we
				// attach debugger to an already started process - it successfully handles all exceptions, but when
				// we restart process, it's raises exception that crash it again
				// I decided to check EXIT_PROCESS_DEBUG_EVENT with abnormal exit code and RIP_EVENT for restart
				if (dEvent.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT)
				{
					Logger::warning(PROC_FATAL_ERROR, manager, 0);
					DebugActiveProcessStop(manager->pId);
					if (exit_code != ProcessManager::EXIT_CODE)
					{
						cout << ProcessManager::getTime() << " unexpected process termination - process" << manager->pId << endl;

						manager->restartProcess();
					}
					return;
				}
				if (dEvent.dwDebugEventCode == RIP_EVENT)
				{
					Logger::warning(PROC_FATAL_ERROR, manager, 0);
					DebugActiveProcessStop(manager->pId);
					cout << ProcessManager::getTime() << " unexpected process termination - process" << manager->pId << endl;

					manager->restartProcess();
					return;
				}
				
				ContinueDebugEvent(dEvent.dwProcessId, dEvent.dwThreadId, DBG_EXCEPTION_NOT_HANDLED);
				
			}

		}
	}	
	else
	{
		Logger::warning(DEB_ATTACH_FAIL, manager, GetLastError());
		cout << ProcessManager::getTime() << " Cannot attach debuger to process " << manager->pId << endl;

		while (1)
		{
			DWORD exit_code=DWORD();
			GetExitCodeProcess(manager->getProcessHandle(), &exit_code);
			if (exit_code != STILL_ACTIVE)
			{
				if (exit_code != ProcessManager::EXIT_CODE)
				{
					cout << ProcessManager::getTime() << " unexpected process termination - process" << manager->pId << endl;
					Logger::warning(PROC_FATAL_ERROR, manager, 0);
					manager->restartProcess();
					break;
				} 
			} 
		}
	}
}

void ProcessManager::getProcessInfo()
{
	DWORD EXIT_INFO = DWORD(999);
	DWORD sz = DWORD(200);
	LPWSTR _exec = new WCHAR[100];
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, pId);
	if (hProcess != NULL)
	{
		QueryFullProcessImageName(hProcess, 0, _exec, &sz);

		GetExitCodeProcess(hProcess, &EXIT_INFO);
		LPWSTR status;
		if (EXIT_INFO == STILL_ACTIVE)
		{
			if (!this->isSuspended)
				status = L"ACTIVE";
			else
				status = L"SUSPENDED";
		} else	status = L"INACTIVE";
		printf("%s Process id:%i\t%S\t%S\n", getTime(), this->pId, _exec, status);

	}
	else
	{
		Logger::warning(PROC_ACCESS_DENIED, this, GetLastError());
	}
}

LPWSTR ProcessManager::getProcessPath()
{
	LPWSTR path = new WCHAR[400];
	DWORD sz = DWORD(400);
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, this->pId);
	QueryFullProcessImageName(hProcess, 0, path, &sz);

	return path;
}

BOOL ProcessManager::GetProcessList()
{
	HANDLE hProcessSnap;
	HANDLE hProcess;
	PROCESSENTRY32 pe32;
	DWORD dwPriorityClass;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return(FALSE);
	}

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof(PROCESSENTRY32);

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);          // clean the snapshot object
	}

	do
	{
		dwPriorityClass = 0;
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
		DWORD sz = DWORD(400);
		LPWSTR sCmdLineInfo = new WCHAR[200];
		QueryFullProcessImageName(hProcess, 0, sCmdLineInfo, &sz);
		
		PWSTR buf;

		if (hProcess == NULL)
		{
			cout << "access denied" <<GetLastError()<< endl;
		}
		else
		{
			get_cmd_line(pe32.th32ProcessID, buf);
			
			_tprintf(TEXT("\nPROCESS NAME:  %i	%s\n%s"), pe32.th32ProcessID, sCmdLineInfo, buf);

			dwPriorityClass = GetPriorityClass(hProcess);
			CloseHandle(hProcess);
		}
	} while (Process32Next(hProcessSnap, &pe32));

	cout << GetCurrentProcessId() << endl;
	CloseHandle(hProcessSnap);
	return(TRUE);
}

ProcessManager::~ProcessManager()
{
	WaitForSingleObject(this->getProcessHandle(),INFINITE);
}
