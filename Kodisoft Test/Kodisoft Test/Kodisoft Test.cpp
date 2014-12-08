#include"stdafx.h"
#include"ProcessManager.h"
//  Forward declarations:

void func()
{

}

int main(void)
{
	setlocale(LC_ALL, "RUS");
	LPWSTR l = LPWSTR(L"C:\\Users\\roman\\AppData\\Roaming\\uTorrent\\uTorrent.exe");
	LPWSTR l2 = LPWSTR(L"");
	LPCWSTR msg = LPCWSTR(L"c:/windows/notepad.exe l.txt");

	ProcessManager * m = new ProcessManager(l, l2);
	//ProcessManager * m = new ProcessManager();
	
	m->startProcess();
	m->getProcessInfo();
	Sleep(10000);

	m->pauseProcess();
	Sleep(10000);

	m->resumeProcess();
	Sleep(10000);
	m->stopProcess();
	WaitForSingleObject(m->getProcessHandle(), INFINITE);
	//_tsystem(L"pause");
	return 0;
}


