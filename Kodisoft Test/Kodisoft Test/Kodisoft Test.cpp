#include"stdafx.h"
#include"ProcessManager.h"
//  Forward declarations:

void func()
{

}

int main(void)
{
	setlocale(LC_ALL, "RUS");
	LPWSTR l = LPWSTR(L"c:/windows/notepad.exe");
	LPWSTR l2 = LPWSTR(L"setuperr");
	//LPCWSTR l2 = LPCWSTR(L"c:/windows/notepad.exe l.txt");

	//ProcessManager *m = new ProcessManager(l, l2);
	ProcessManager *m = new ProcessManager(7316);
	//m->startProcess();
//	ProcessManager::GetProcessList();
	while (WaitForSingleObject(m->getProcessHandle(), 0))
	{
		if (!GetCurrentProcess)
			m->stopProcess();
	};
	_tsystem(L"pause");
	return 0;
}


