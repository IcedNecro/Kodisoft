#pragma once

/**
 * ProcessManager - class I may to realize. It can perform basic operations 
 * with processes: start, suspend, resume, stop processes. Also, it can handle
 * process crash and watch it's state.
 * Class supports user calback functions. 
 * Class supports two ways to work with processes
 * <ul>
 *	<li><b>Open new process with specified path and arguments.</b> Creates new process 
 *		and handles it's state. Notice that some processes may not allow you to start
 *		or perform any another operation with them. Class hasn't any access to system
 *		processes, so don't be wondered if it wouldn't work with some processes
 *  </li>
 *  <li><b>Make class to be able to handle already opened process</b> Specify id of 
 *		currently running process. As I noticed earlier, class has some permissions of 
 *		access.
 *  </li>
 * </ul>
 * Handling process crash is made with Debug Api (WinApi). Some processes don't supports
 * debugging (system processes, or simple processes such as notepad.exe or calc.exe). 
 * So in this cases for restarting process I used GetExitCodeProcess function to recieve
 * exit code :(
 * @author - Roman Statkevich, Kyiv Polytechic Insitute
 * Contact me by email romanstatkevich@gmail.com
 */
class ProcessManager
{
	friend class Logger;
	
	thread * t;
	
	int isSuspended = -1;
	

	WORD pId;

	wstring path;
	
	function<void(ProcessManager *)> onStop;
	function<void(ProcessManager *)> onStart;
	function<void(ProcessManager *)> onResume;
	function<void(ProcessManager *)> onPause;

	/** 
	 * This function is used into debugging thread for watching process crashes
	 * For process monitoring DebugAPI features were used. Unfortunaly, not all 
	 * processes could be handled by debugger. So, for such processes exists 
	 * another way to handle it. For every loop I checks exit code for this 
	 * process. If it's not equal to 0 or STILL_ACTIVE(249) - it means that
	 * process finnished abnormally
	 */
	static void debugLoop(ProcessManager*);

	/**
	 * @return - formatted date string, used in cinsole notifications
	 */
	static char* getTime();
	
public:
	/**
	 * Constructor that is defined for creating new process via CreateProcess()
	 * To start process, you also have to call ProcessManager::startProcess() 
	 * method, that also starts debugging thread for process you started 
	 * @arg _cmd_line_path - path to the process executable in your file system
	 * @arg _args - comand line arguments to pass to process
	 */
	ProcessManager(LPWSTR, LPWSTR);

	/**
	 * Constructor, that opens currently running process by pId passed into it
	 * It starts debugging thread for opened process
	 * @arg pId - ID of process you want to open and monitor
	 */
	ProcessManager(DWORD);

	/**
	 * Default constructor
	 */
	ProcessManager();

	/**
	 * Method returns string with process path in file system
	 * @return path of process executable in local file system
	 */
	LPWSTR getProcessPath();
	
	/**
	 */
	DWORD getRunningThreadInfo();
	
	/**
	 * Methods suspends all threads, that belong to current process, if current
	 * process is running
	 */
	bool pauseProcess();
	
	/**
	 * Method resumes all threads of current process, if current process 
	 * is suspended
	 */
	bool resumeProcess();
	
	/**
	 * Method terminates process
	 */
	bool stopProcess();
	
	/**
	 * Method restarts process with the same command line as it had started before
	 */
	bool restartProcess();
	
	/**
	 * Output information about process (executable, current state, process id)
	 */
	void getProcessInfo();

	bool reopenProcess();
	
	/**
	 * @return handle of process being monitored by class
	 */
	HANDLE& getProcessHandle();
	/**
	 * Starts process with command line arguments, that were defined in constructor.
	 * Starts monitoring of process crash
	 */
	bool startProcess();
	
	/**
	 * Sets function that is called when process stopped
	 * @arg func - pointer to function that would be set as callback function when
	 * process terminates by method call {@code stopProcess()}. func prototype must look
	 * like this:
	 * {@code void func(ProcessManager *ptr)}
	 * <i>ptr</i> allows you to perform some operations you want with your manager in your 
	 * callback function, such as logging, watching manager and process state etc.
	 */
	void setOnProcessStopListener(void(*func)(ProcessManager *));
	
	/**
	 * Sets function that is called when process resumed
	 * @arg func - pointer to function that would be set as callback function when
	 * process resumes by method call {@code resumeProcess()}. func prototype must look
	 * like this:
	 * {@code void func(ProcessManager *ptr)}
	 * <i>ptr</i> allows you to perform some operations you want with your manager in your
	 * callback function, such as logging, watching manager and process state etc.
	 */
	void setOnProcessResumeListener(void(*func)(ProcessManager *));
	
	/**
	 * Sets function that is called when process paused
	 * @arg func - pointer to function that would be set as callback function when
	 * process pauses by method call {@code pauseProcess()}. func prototype must look
	 * like this:
	 * {@code void func(ProcessManager *ptr)}
	 * <i>ptr</i> allows you to perform some operations you want with your manager in your
	 * callback function, such as logging, watching manager and process state etc.
	 */
	void setOnProcessPauseListener(void(*func)(ProcessManager *));
	
	/**
	 * Sets function that is called when process started
	 * @arg func - pointer to function that would be set as callback function when
	 * process starts by method call {@code startProcess()}. func prototype must look
	 * like this:
	 * {@code void func(ProcessManager *ptr)}
	 * <i>ptr</i> allows you to perform some operations you want with your manager in your
	 * callback function, such as logging, watching manager and process state etc.
	 */
	void setOnProcessStartListener(void(*func)(ProcessManager *));
	
	/**
	 * Shows the list of currently running processes with it's executables, command lines
	 * and pID's
	 */
	static BOOL GetProcessList();

	~ProcessManager();
};

