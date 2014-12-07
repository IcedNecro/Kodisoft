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

// PROCESS_BASIC_INFORMATION for pure 32 and 64-bit processes
typedef struct _PROCESS_BASIC_INFORMATION {
	PVOID Reserved1;
	PVOID PebBaseAddress;
	PVOID Reserved2[2];
	ULONG_PTR UniqueProcessId;
	PVOID Reserved3;
} PROCESS_BASIC_INFORMATION;

// PROCESS_BASIC_INFORMATION for 32-bit process on WOW64
// The definition is quite funky, as we just lazily doubled sizes to match offsets...
typedef struct _PROCESS_BASIC_INFORMATION_WOW64 {
	PVOID Reserved1[2];
	PVOID64 PebBaseAddress;
	PVOID Reserved2[4];
	ULONG_PTR UniqueProcessId[2];
	PVOID Reserved3[2];
} PROCESS_BASIC_INFORMATION_WOW64;

typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING;

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

	// determine if this process is running on WOW64
	BOOL wow;
	IsWow64Process(GetCurrentProcess(), &wow);

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

	try
	{
		if (wow)
		{
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
		}
		else
		{
			// we're running as a 32-bit process in a 32-bit OS, or as a 64-bit process in a 64-bit OS
			PROCESS_BASIC_INFORMATION pbi;
			ZeroMemory(&pbi, sizeof(pbi));

			// get process information
			_NtQueryInformationProcess query = (_NtQueryInformationProcess)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");
			err = query(hProcess, 0, &pbi, sizeof(pbi), NULL);
			if (!err)
			{
				printf("NtQueryInformationProcess failed\n");
				CloseHandle(hProcess);
				return GetLastError();
			}

			// read PEB
			if (!ReadProcessMemory(hProcess, pbi.PebBaseAddress, peb, pebSize, NULL))
			{
				printf("ReadProcessMemory PEB failed\n");
				CloseHandle(hProcess);
				return GetLastError();
			}

			// read ProcessParameters
			PBYTE* parameters = (PBYTE*)*(LPVOID*)(peb + ProcessParametersOffset); // address in remote process adress space
			if (!ReadProcessMemory(hProcess, parameters, pp, ppSize, NULL))
			{
				printf("ReadProcessMemory Parameters failed\n");
				CloseHandle(hProcess);
				return GetLastError();
			}

			// read CommandLine
			UNICODE_STRING* pCommandLine = (UNICODE_STRING*)(pp + CommandLineOffset);
			cmdLine = (PWSTR)malloc(pCommandLine->MaximumLength);
			if (!ReadProcessMemory(hProcess, pCommandLine->Buffer, cmdLine, pCommandLine->MaximumLength, NULL))
			{
				printf("ReadProcessMemory Parameters failed\n");
				CloseHandle(hProcess);
				return GetLastError();
			}
		}

		buf = cmdLine;
	}
	catch (exception())
	{
		cout << "command line function fails" << endl;
	}

}


void ProcessManager::pauseProcess()
{

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, this->pId);
	
//	if (!this->isSuspended)
	{

		HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

		THREADENTRY32 threadEntry;
		threadEntry.dwSize = sizeof(THREADENTRY32);

		Thread32First(hThreadSnapshot, &threadEntry);

		BOOL allThreadsRunning = true;

		do
		{
			if (threadEntry.th32OwnerProcessID == this->pId)
			{
				HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE,
					threadEntry.th32ThreadID);
				DWORD val = SuspendThread(hThread);
				if (val != 0)
					ResumeThread(hThread);
				else
					this->isSuspended = true;
				CloseHandle(hThread);
			}
		} while (Thread32Next(hThreadSnapshot, &threadEntry));

		CloseHandle(hThreadSnapshot);
		{
			this->onPause(this);
			this->isSuspended = true;
		}
	}/* else
	{
		cout << "current process is already suspended" << endl;
	}*/

}

void ProcessManager::resumeProcess()
{
	//if (this->isSuspended)
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
		//else
		{
			if (val <= 1)
				this->isSuspended = false;

			this->onResume(this);
		}
	}
	 /*else 
	 {
		 cout << "current process is already resumed" << endl;
	 }*/
	
}

void ProcessManager::stopProcess()
{
	UINT exit_code = 0x0000042;
	cout << "LOL " << exit_code << endl;
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, this->pId);
	if (!TerminateProcess(hProcess, exit_code))
		cout<<"err"<<endl;
	else
	{
		this->onStop(this);
		//Logger::log(PROC_STOP_EVENT, this);
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

void ProcessManager::restartProcess()
{
	STARTUPINFO cif;
	PROCESS_INFORMATION pi;
	ZeroMemory(&cif, sizeof(STARTUPINFO));
	this->isSuspended = false;
	if (!CreateProcess(NULL, LPWSTR(this->path.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &cif, &pi))
	{
		Logger::error(PROC_FAILED_WHILE_STARTED, this, GetLastError());
		cout << GetLastError() << " error " << GetLastError() << endl;
	}
	else
	{
		this->pId = DWORD(pi.dwProcessId);
		t->detach();
		delete t;
		t = new thread(&ProcessManager::getInfo, this);

		this->onStart(this);
	}
}

void ProcessManager::startProcess()
{
	STARTUPINFO cif;
	PROCESS_INFORMATION pi;
	ZeroMemory(&cif, sizeof(STARTUPINFO));
	this->isSuspended = false;
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, this->pId);

	DWORD exit_code = DWORD(1);
	GetExitCodeProcess(hProcess, &exit_code);

	if (!CreateProcess(NULL, LPWSTR(this->path.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &cif, &pi) && exit_code!=STILL_ACTIVE)
	{
		Logger::error(PROC_FAILED_WHILE_STARTED, this, GetLastError());
		cout << GetLastError() << " error " << GetLastError() << endl;
	}
	else
	{
		this->pId = DWORD(pi.dwProcessId);
		
		t = new thread(&ProcessManager::getInfo, this);
		
		this->onStart(this);
	}
}

bool ProcessManager::reopenProcess()
{

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, this->pId);
	return true;
}

ProcessManager::ProcessManager(LPWSTR _cmd_prompt_path, LPWSTR _args)
{
	this->path = wstring(_cmd_prompt_path) + wstring(L" ") + wstring(_args);

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

ProcessManager::ProcessManager(DWORD pId)
{ 
	HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, false, pId);

	if (handle == NULL)
	{
		DWORD err = GetLastError();

		if (err == ERROR_ACCESS_DENIED)
		{
			Logger::log(PROC_ACCESS_DENIED, this);
		}
		else
		{
			Logger::log(PROC_FAILED_WHILE_OPEN, this);
		}
	}
	else
	{
		PWSTR buf;
		get_cmd_line(pId, buf);
		this->path = buf;
		this->pId = pId;
		this->isSuspended = false;

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
		t = new thread(ProcessManager::getInfo, this);
	}
}

HANDLE& ProcessManager::getProcessHandle()
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, pId);

	return hProcess;
}

void ProcessManager::getInfo(ProcessManager *manager)
{
	if (DebugActiveProcess(manager->pId))
	{
		Logger::log(DEB_ATTACH_SUCCESS, manager);
		while (1)
		{
			DWORD exit_code;
			DEBUG_EVENT dEvent;

			if (WaitForDebugEvent(&dEvent, INFINITE))
			{
				ContinueDebugEvent(dEvent.dwProcessId, dEvent.dwThreadId, DBG_CONTINUE);
				if (dEvent.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT)
				{
					HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, manager->pId);
					Logger::error(DEB_APP_FAILURE, manager);
					DebugBreakProcess(hProcess);

					manager->restartProcess();
					break;
				}
			}
		}
	}	
	else
		Logger::log(DEB_ATTACH_FAIL, manager);

}

void ProcessManager::getProcessInfo()
{
	DWORD EXIT_INFO = DWORD(999);

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, pId);

	cout << "Process id:\t" << this->pId << endl;
	GetExitCodeProcess(hProcess, &EXIT_INFO);
	cout << "exit:" << EXIT_INFO << endl;
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
	//	cout <<"\nerror code:"<< GetLastError() << endl;
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

	cout << "Destroing class" << endl;
//	this->stopProcess();
//	this->t.detach();
}
