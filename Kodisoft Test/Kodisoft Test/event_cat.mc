
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
MessageId=0x203
Severity=Warning
Facility=System
SymbolicName=PROC_ACCESS_DENIED
Language=English
Process %1 starting failed
.
;