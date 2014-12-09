#include "Process.h"

// This module is actually not mine. I've founded it here http://www.cyberforum.ru/post4909508.html
// As I understand, getting command line is a little bit difficult task
// Only way to get arguments of command line is to execute PEB structure
// But it's position in memory depends from bittness of system (x64 or x86),
// processor type, bittness of processes etc.
// I've tried to change something, but I discovered that all fragments of code
// are nessecary to execute command lines from almost all processes. So I decided 
// not to change anything

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
	try
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
