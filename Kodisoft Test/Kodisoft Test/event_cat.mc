
SeverityNames=(Success=0x0:STATUS_SEVERITY_SUCCESS
               Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
               Warning=0x2:STATUS_SEVERITY_WARNING
               Error=0x3:STATUS_SEVERITY_ERROR
              )


FacilityNames=(   System=0x0:FACILITY_SYSTEM 
				  Runtime=0x1:FACILITY_RUNTIME)

LanguageNames=(English=0x409:MSG00409)

; // Message categories are defined here

MessageIdTypedef=WORD

MessageId=0x1
SymbolicName=PROCESS_MANAGER_LOG
Language=English
Category 1
.
MessageId=0x2
SymbolicName=PROCESS_MANAGER_ERRORS
Language=English
Category 2
.
MessageId=0x3
SymbolicName=DEBUG_EVENTS
Language=English
Category 2
.
; // The following are the message definitions.
MessageIdTypedef=DWORD

MessageId=0x100
Severity=Success
Facility=Runtime
SymbolicName=PROC_SUCCESSFULLY_STARTED
Language=English
Process %1 started
.
MessageId=0x101
Severity=Success
Facility=Runtime
SymbolicName=PROC_SUSPEND_EVENT
Language=English
Process %1 suspended
.
MessageId=0x102
Severity=Success
Facility=Runtime
SymbolicName=PROC_RESUME_EVENT
Language=English
Process %1 resumed
.
MessageId=0x103
Severity=Success
Facility=Runtime
SymbolicName=PROC_STOP_EVENT
Language=English
Process %1 stopped
.
MessageId=0x104
Severity=Success
Facility=Runtime
SymbolicName=PROC_RESTART_EVENT
Language=English
Process %1 restarted 
.
MessageId=0x105
Severity=Success
Facility=Runtime
SymbolicName=PROC_OPEN_EVENT
Language=English
Process %1 opened by id %2
.
MessageId=0x200
Severity=Error
Facility=Runtime
SymbolicName=PROC_FAILED_WHILE_STARTED
Language=English
Process %1 starting failed
.
MessageId=0x201
Severity=Error
Facility=Runtime
SymbolicName=PROC_FATAL_ERROR
Language=English
Unexpected termination of process %1
.
MessageId=0x202
Severity=Error
Facility=Runtime
SymbolicName=PROC_FAILED_WHILE_OPEN
Language=English
Process %1 opened by id %2
.
MessageId=0x203
Severity=Warning
Facility=System
SymbolicName=PROC_ACCESS_DENIED
Language=English
Process %1 is under permission
.
MessageId=0x300
Severity=Error
Facility=Runtime
SymbolicName=DEB_ATTACH_FAIL
Language=English
Debugger attach to %1 process failed
.
MessageId=0x301
Severity=Success
Facility=Runtime
SymbolicName=DEB_ATTACH_SUCCESS
Language=English
Debugger attached to %1 process
.
MessageId=0x302
Severity=Error
Facility=Runtime
SymbolicName=DEB_APP_FAILURE
Language=English
Process %1 restarting after failure
.
;