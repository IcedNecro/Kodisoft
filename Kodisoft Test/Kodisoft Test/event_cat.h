 // Message categories are defined here
//
//  Values are 32 bit values laid out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//
#define FACILITY_SYSTEM                  0x0
#define FACILITY_RUNTIME                 0x1


//
// Define the severity codes
//
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


//
// MessageId: PROCESS_MANAGER_LOG
//
// MessageText:
//
// Category 1
//
#define PROCESS_MANAGER_LOG              ((WORD)0x00000001L)

//
// MessageId: PROCESS_MANAGER_ERRORS
//
// MessageText:
//
// Category 2
//
#define PROCESS_MANAGER_ERRORS           ((WORD)0x00000002L)

 // The following are the message definitions.
//
// MessageId: PROC_SUCCESSFULLY_STARTED
//
// MessageText:
//
// Process %1 started
//
#define PROC_SUCCESSFULLY_STARTED        ((DWORD)0x00010100L)

//
// MessageId: PROC_SUSPEND_EVENT
//
// MessageText:
//
// Process %1 suspended
//
#define PROC_SUSPEND_EVENT               ((DWORD)0x00010101L)

//
// MessageId: PROC_RESUME_EVENT
//
// MessageText:
//
// Process %1 resumed
//
#define PROC_RESUME_EVENT                ((DWORD)0x00010102L)

//
// MessageId: PROC_STOP_EVENT
//
// MessageText:
//
// Process %1 stopped
//
#define PROC_STOP_EVENT                  ((DWORD)0x00010103L)

//
// MessageId: PROC_FAILED_WHILE_STARTED
//
// MessageText:
//
// Process %1 starting failed
//
#define PROC_FAILED_WHILE_STARTED        ((DWORD)0xC0010200L)

//
// MessageId: PROC_FATAL_ERROR
//
// MessageText:
//
// Unexpected termination of process %1
//
#define PROC_FATAL_ERROR                 ((DWORD)0xC0010201L)

//
// MessageId: PROC_ACCESS_DENIED
//
// MessageText:
//
// Process %1 starting failed
//
#define PROC_ACCESS_DENIED               ((DWORD)0x80000203L)

