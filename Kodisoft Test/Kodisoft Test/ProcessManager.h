

#pragma once
class ProcessManager
{
	friend class Logger;
	thread t;
	fstream f;
	int isSuspended = -1;
	
	LPWSTR path;
	LPWSTR args;

	function<void(ProcessManager *)> onStop;
	function<void(ProcessManager *)> onStart;
	function<void(ProcessManager *)> onResume;
	function<void(ProcessManager *)> onPause;

	STARTUPINFO cif;
	PROCESS_INFORMATION pi;
	HANDLE handler;

	static void getInfo(ProcessManager*);
	
public:
	ProcessManager(LPWSTR, LPWSTR);
	ProcessManager();

	DWORD getRunningThreadInfo();
	void pauseProcess();
	void resumeProcess();
	void stopProcess();
	void restartProcess();
	void getProcessInfo();
	bool reopenProcess();
	HANDLE& getThreadHandle();
	HANDLE& getProcessHandle();
	void startProcess();
	void setOnProcessStopListener(void(*func)(ProcessManager *));
	void setOnProcessResumeListener(void(*func)(ProcessManager *));
	void setOnProcessPauseListener(void(*func)(ProcessManager *));
	void setOnProcessStartListener(void(*func)(ProcessManager *));
	static BOOL GetProcessList();

	~ProcessManager();
};

