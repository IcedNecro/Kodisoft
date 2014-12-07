#include"stdafx.h"
#include"ProcessManager.h"
//  Forward declarations:

void func()
{

}

int main(void)
{
	setlocale(LC_ALL, "RUS");
	LPWSTR l = LPWSTR(L"d:/Project1.exe");
	LPWSTR l2 = LPWSTR(L"");
	LPCWSTR msg = LPCWSTR(L"c:/windows/notepad.exe l.txt");

	//ProcessManager *m = new ProcessManager(l, l2);
	ProcessManager *m = new ProcessManager(4012);
	
	m->startProcess();
	//Sleep(1000);
	//m->pauseProcess();
	//Sleep(2000);
	//m->resumeProcess();
//	m->stopProcess();
//	ProcessManager::GetProcessList();
/*	while (WaitForSingleObject(m->getProcessHandle(), 0))
	{
		if (!GetCurrentProcess)
			m->stopProcess();
	};*/
	/*Sleep(5000);
	cout << "asleep" << endl;
	m->pauseProcess();
	Sleep(5000);
	cout << "resumed" << endl;
	m->resumeProcess();
	Sleep(5000);
	cout << "finished" << endl;*/
	_tsystem(L"pause");
	return 0;
}


