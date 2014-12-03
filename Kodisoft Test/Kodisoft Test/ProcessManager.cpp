#include "stdafx.h"
#include "ProcessManager.h"
#include "Logger.h"

// NtQueryInformationProcess for pure 32 and 64-bit processes
typedef NTSTATUS(NTAPI *_NtQueryInformationProcess)(
	IN HANDLE ProcessHandle,
	ULONG ProcessInformationClass,
	OUT PVOID ProcessInformation,
	IN ULONG ProcessInformationLength,
	OUT PULONG ReturnLength OPTIONAL
	);

typedef NTSTATUS(NTAPI *_NtReadVirtualMemory)(
	IN HANDLE ProcessHandle,
	IN PVOID BaseAddress,
	OUT PVOID Buffer,
	IN SIZE_T Size,
	OUT PSIZE_T NumberOfBytesRead);

// NtQueryInformationProcess for 32-bit process on WOW64
typedef NTSTATUS(NTAPI *_NtWow64ReadVirtualMemory64)(
	IN HANDLE ProcessHandle,
	IN PVOID64 BaseAddress,
	OUT PVOID Buffer,
	IN ULONG64 Size,
	OUT PULONG64 NumberOfBytesRead);
/*
// PROCESS_BASIC_INFORMATION for pure 32 and 64-bit processes
typedef struct _PROCESS_BASIC_INFORMATION {
	PVOID Reserved1;
	PVOID PebBaseAddress;
	PVOID Reserved2[2];
	ULONG_PTR UniqueProcessId;
	PVOID Reserved3;
} PROCESS_BASIC_INFORMATION;
*/
// PROCESS_BASIC_INFORMATION for 32-bit process on WOW64
// The definition is quite funky, as we just lazily doubled sizes to match offsets...
typedef struct _PROCESS_BASIC_INFORMATION_WOW64 {
	PVOID Reserved1[2];
	PVOID64 PebBaseAddress;
	PVOID Reserved2[4];
	ULONG_PTR UniqueProcessId[2];
	PVOID Reserved3[2];
} PROCESS_BASIC_INFORMATION_WOW64;
/*
typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING;
*/
typedef struct _UNICODE_STRING_WOW64 {
	USHORT Length;
	USHORT MaximumLength;
	PVOID64 Buffer;
} UNICODE_STRING_WOW64;

int get_cmd_line(DWORD dwId, PWSTR &buf)
{
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwId);
	DWORD err = 0;
	if (hProcess == NULL)
	{
		printf("OpenProcess %u failed\n", dwId);
		return GetLastError();
	}

	// determine if 64 or 32-bit processor
	SYSTEM_INFO si;
	GetNativeSystemInfo(&si);

	// use WinDbg "dt ntdll!_PEB" command and search for ProcessParameters offset to find the truth out
	DWORD ProcessParametersOffset = si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? 0x20 : 0x10;
	DWORD CommandLineOffset = si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? 0x70 : 0x40;

	// read basic info to get ProcessParameters address, we only need the beginning of PEB
	DWORD pebSize = ProcessParametersOffset + 8;
	PBYTE peb = (PBYTE)malloc(pebSize);
	ZeroMemory(peb, pebSize);

	// read basic info to get CommandLine address, we only need the beginning of ProcessParameters
	DWORD ppSize = CommandLineOffset + 16;
	PBYTE pp = (PBYTE)malloc(ppSize);
	ZeroMemory(peb, pebSize);

	PWSTR cmdLine;

		// we're running as a 32-bit process in a 64-bit OS
		PROCESS_BASIC_INFORMATION_WOW64 pbi;
		ZeroMemory(&pbi, sizeof(pbi));

		// get process information from 64-bit world
		_NtQueryInformationProcess query = (_NtQueryInformationProcess)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtWow64QueryInformationProcess64");
		err = query(hProcess, 0, &pbi, sizeof(pbi), NULL);
		if (err != 0)
		{
			printf("NtWow64QueryInformationProcess64 failed\n");
			CloseHandle(hProcess);
			return GetLastError();
		}

		// read PEB from 64-bit address space
		_NtWow64ReadVirtualMemory64 read = (_NtWow64ReadVirtualMemory64)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtWow64ReadVirtualMemory64");
		err = read(hProcess, pbi.PebBaseAddress, peb, pebSize, NULL);
		if (err != 0)
		{
			printf("NtWow64ReadVirtualMemory64 PEB failed\n");
			CloseHandle(hProcess);
			return GetLastError();
		}

		// read ProcessParameters from 64-bit address space
		PBYTE* parameters = (PBYTE*)*(LPVOID*)(peb + ProcessParametersOffset); // address in remote process adress space
		err = read(hProcess, parameters, pp, ppSize, NULL);
		if (err != 0)
		{
			printf("NtWow64ReadVirtualMemory64 Parameters failed\n");
			CloseHandle(hProcess);
			return GetLastError();
		}
		// read CommandLine
		UNICODE_STRING_WOW64* pCommandLine = (UNICODE_STRING_WOW64*)(pp + CommandLineOffset);
		cmdLine = (PWSTR)malloc(pCommandLine->MaximumLength);
		err = read(hProcess, pCommandLine->Buffer, cmdLine, pCommandLine->MaximumLength, NULL);
		if (err != 0)
		{
			printf("NtWow64ReadVirtualMemory64 Parameters failed\n");
			CloseHandle(hProcess);
			return GetLastError();
		}
	
	buf = cmdLine;
}

void ProcessManager::pauseProcess()
{
	if (!this->isSuspended)
	{
		if(!SuspendThread(this->pi.hThread))
			;
		else
		{
			this->onPause(this);
			this->isSuspended = true;
		}
	} else
	{
		cout << "already suspend" << endl;
	}

}

void ProcessManager::resumeProcess()
{
	if (this->isSuspended)
	{
		if(!ResumeThread(this->pi.hThread))
			;
		else
		{
			this->onResume(this);
			this->isSuspended = false;
		}
	}
	 else 
	 {
		 cout << "already resumed" << endl;
	 }
	
}

void ProcessManager::stopProcess()
{
	UINT exit_code = 0x0000042;
	this->onStop(this);
	if (!TerminateProcess(this->pi.hProcess, exit_code))
		;
	else
		Logger::log(PROC_STOP_EVENT, this);
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

void ProcessManager::restartProcess()
{
	wstring commandLine = wstring(this->path) + wstring(L" ") + wstring(this->args);
	if (!CreateProcess(NULL, LPWSTR(commandLine.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &cif, &pi))
		cout << "internal error" << endl;
	else
		cout << "connect" << endl;
}

void ProcessManager::startProcess()
{
//	WCHAR *commandLineContents;

//	Logger log(L"log.txt", this);
	ZeroMemory(&cif, sizeof(STARTUPINFO));
	this->isSuspended = false;
	wstring commandLine = wstring(this->path) + wstring(L" ") + wstring(this->args);
	if (!CreateProcess(NULL, LPWSTR(commandLine.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &cif, &pi))
	{
		Logger::error(PROC_FAILED_WHILE_STARTED, this, GetLastError());
	}
	else
	{
		t = thread(&ProcessManager::getInfo, this);

		this->onStart(this);
	}
}

bool ProcessManager::reopenProcess()
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, this->pi.dwProcessId);
	return true;
}

ProcessManager::ProcessManager(LPWSTR _cmd_prompt_path, LPWSTR _args)
{
	this->path = _cmd_prompt_path;
	this->args = _args;

	this->onStop = [](ProcessManager * pm)
	{
		Logger::log(PROC_STOP_EVENT, pm);
	};

	this->onResume = [](ProcessManager * pm)
	{
		Logger::log(PROC_RESUME_EVENT, pm);
	};

	this->onStart = [](ProcessManager * pm)
	{
		Logger::log(PROC_SUCCESSFULLY_STARTED, pm);
	};

	this->onPause = [](ProcessManager * pm)
	{
		Logger::log(PROC_SUSPEND_EVENT, pm);
	}; 
}

HANDLE& ProcessManager::getThreadHandle()
{
	return this->pi.hThread;
}

HANDLE& ProcessManager::getProcessHandle()
{
	return this->pi.hProcess;
}

void ProcessManager::getInfo(ProcessManager *manager)
{
	while (1)
	{
		DWORD exit_code;
		GetExitCodeProcess(manager->getProcessHandle(), &exit_code);
		if (exit_code == 0)
		{
			cout << "Application crash!" << endl;
			manager->restartProcess();
		}
		Sleep(1000);
		cout << exit_code << endl;
	}
}

void ProcessManager::getProcessInfo()
{
	DWORD EXIT_INFO = 999;
	cout << "Process id:\t" << this->pi.dwProcessId << endl
		<< "Thread id:\t" << this->pi.dwThreadId << endl;
	GetExitCodeProcess(this->pi.hProcess, &EXIT_INFO);
	cout<< "Exit code:" << GetExitCodeThread(this->pi.hProcess, &EXIT_INFO);
	if (EXIT_INFO == STILL_ACTIVE)
	{
		if (!this->isSuspended)
			cout << "is active";
		else
			cout << "is suspended" << endl;
	}
	else
		cout << "is innactive" << endl;
	
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

		LPWSTR sCmdLineInfo = new WCHAR[200];
		cout <<"\nerror code:"<< GetLastError() << endl;
		GetProcessImageFileName(hProcess, sCmdLineInfo, 200);

		PWSTR buf;

		get_cmd_line(pe32.th32ProcessID, buf);

		_tprintf(TEXT("\nPROCESS NAME:  %i	%s\n"), pe32.th32ProcessID, pe32.szExeFile);

		dwPriorityClass = GetPriorityClass(hProcess);
		CloseHandle(hProcess);
	} while (Process32Next(hProcessSnap, &pe32));

	cout << GetCurrentProcessId() << endl;
	CloseHandle(hProcessSnap);
	return(TRUE);
}


ProcessManager::~ProcessManager()
{
	CloseHandle(this->pi.hProcess);
	CloseHandle(this->pi.hThread);
	cout << "Destroing class" << endl;
	this->t.detach();
	this->stopProcess();
}
