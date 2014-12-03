#include"stdafx.h"
#include"ProcessManager.h"
//  Forward declarations:

void func()
{

}

int main(void)
{
	setlocale(LC_ALL, "RUS");
	LPWSTR l = LPWSTR(L"c:/windows/notepad.ex");
	LPWSTR l2 = LPWSTR(L"setuperr");
	//LPCWSTR l2 = LPCWSTR(L"c:/windows/notepad.exe l.txt");

	ProcessManager *m = new ProcessManager(l, l2);
	m->startProcess();
	//Sleep(10000);
	//ProcessManager m2(l2);
	//ProcessManager::GetProcessList();
	//m->stopProcess();
	m->stopProcess();
	_tsystem(L"pause");
	return 0;
}


