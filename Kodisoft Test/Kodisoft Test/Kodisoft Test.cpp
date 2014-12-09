#include"stdafx.h"
#include"ProcessManager.h"
//  Forward declarations:



void func()
{

}

int main(void)
{
	setlocale(LC_ALL, "RUS");
	LPWSTR msg = LPWSTR(L"c:/windows/notepad.exe");
	LPWSTR l = LPWSTR(L"D:\\Program Files\\sketch\\SketchUp.exe");
	LPWSTR l2 = LPWSTR(L"b.txt");
	//ProcessManager m = ProcessManager(msg, l2);
	ProcessManager m = ProcessManager(756);
	//m.startProcess();
	m.getProcessInfo();
	Sleep(10000);
	m.pauseProcess();
	Sleep(10000);
	m.resumeProcess();
	Sleep(10000);
//	m.stopProcess();
	while (1){}
	//WaitForSingleObject(GetCurrentProcess(), INFINITE);
	return 0;
}


