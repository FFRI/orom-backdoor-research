/*
 * (c) FFRI Security, Inc., 2024 / Author: FFRI Security, Inc.
 */

#include <Windows.h>
#include <evntrace.h>
#include <evntcons.h>
#include <tdh.h>
#include <stdio.h>
#include <stddef.h>

char gSessionName[] = "MyTraceSession";
char gLogFilePath[] = "C:\\Users\\WDKRemoteUser\\Desktop\\MyEtwLog.etl";
GUID ProviderGuid = { 0x22FB2CD6, 0x0E7B, 0x422B, { 0xA0, 0xC7, 0x2F, 0xAD, 0x1F, 0xD0, 0xE7, 0x16 } };  // Microsoft-Windows-Kernel-Process

BOOL gTraceStop = FALSE;


typedef struct _MY_EVENT_TRACE_PROPERTIES {
	EVENT_TRACE_PROPERTIES prop;
	char LogFileName[1024];
	char LoggerName[1024];
} MY_EVENT_TRACE_PROPERTIES;


VOID
EventRecordCallback(
	IN  PEVENT_RECORD  EventRecord
) {
	if (EventRecord->EventHeader.Flags & EVENT_HEADER_FLAG_STRING_ONLY) {
		printf("String: %wZ\n", EventRecord->UserData);
	}
	else {
		TDHSTATUS status;
		PTRACE_EVENT_INFO EventTraceInfo = NULL;
		ULONG EventTraceInfoSz = 0;
		status = TdhGetEventInformation(
			EventRecord,
			0,
			NULL,
			EventTraceInfo,
			&EventTraceInfoSz
		);
		// printf("TdhGetEventInformation: 0x%X\n", status);

		EventTraceInfo = (PTRACE_EVENT_INFO)malloc(EventTraceInfoSz);
		
		status = TdhGetEventInformation(
			EventRecord,
			0,
			NULL,
			EventTraceInfo,
			&EventTraceInfoSz
		);
		// printf("TdhGetEventInformation: 0x%X, EventTraceInfo: 0x%I64X\n", status, EventTraceInfo);
		
		//
		// Parse Information
		//
		PEVENT_PROPERTY_INFO PropInfo = EventTraceInfo->EventPropertyInfoArray;
		LPWSTR TaskName = (LPWSTR)((UINT64)EventTraceInfo + EventTraceInfo->TaskNameOffset);
		for (ULONG i = 0; i < EventTraceInfo->PropertyCount; i++) {
			LPWSTR PropName = (LPWSTR)((UINT64)EventTraceInfo + PropInfo->NameOffset);

			PROPERTY_DATA_DESCRIPTOR PropDesc = { 0 };
			PropDesc.PropertyName = PropName;
			PropDesc.ArrayIndex = ULONG_MAX;
			ULONG PropSize;
			TdhGetPropertySize(
				EventRecord,
				0, NULL,
				1, &PropDesc,
				&PropSize
			);
			PBYTE PropData = malloc(PropSize);
			TdhGetProperty(
				EventRecord,
				0, NULL,
				1, &PropDesc,
				PropSize,
				PropData
			);

			if (!lstrcmpW(PropName, L"ProcessID")) {
				ULONG ProcessID = *(ULONG*)PropData;
				ULONG TargetPid = *(ULONG*)EventRecord->UserContext;
				if (ProcessID == TargetPid) {
					LPWSTR EventMessage = (LPWSTR)((UINT64)EventTraceInfo + EventTraceInfo->EventMessageOffset);
					printf("[PID: %d] (%S) %S\n", ProcessID, TaskName, EventMessage);
					//printf("%s\n", (LPCSTR)((UINT64)EventTraceInfo + EventTraceInfo->BinaryXMLOffset));
				}
				else
					break;
			}

			//
			// ====== Below events are Target Process's event =====
			//
			if (!lstrcmpW(PropName, L"StartAddr")) {
				UINT64 StartAddr = *(UINT64*)PropData;
				printf("\tStartAddr: 0x%I64X\n", StartAddr);
			}

			free(PropData);
			PropInfo++;
		}
	}
}


BOOL
BufferCallback(
	IN  PEVENT_TRACE_LOGFILEA LogFile
) {
	if (gTraceStop)
		return FALSE; // Stop ProcessTrace
	else {
		return TRUE;
	}
}


BOOL
WINAPI
ConsoleCtrlHandler(
	DWORD Signal
) {
	if (Signal == CTRL_C_EVENT)
		gTraceStop = TRUE;
	return TRUE;
}


int
main(
	int  argc,
	char *argv[]
) {
	if (argc != 2) {
		printf("Usage: .\\EtwConsumer.exe <PID>\n");
		return 1;
	}
	ULONG Pid = atoi(argv[1]);
	BOOL res = SetConsoleCtrlHandler(
		ConsoleCtrlHandler,
		TRUE
	);
	printf("SetConsoleCtrlHandler: %d\n", res);

	ULONG ErrCode;
	TRACEHANDLE hTrace;
	MY_EVENT_TRACE_PROPERTIES TraceProp = { 0 };
	TraceProp.prop.Wnode.BufferSize = sizeof(MY_EVENT_TRACE_PROPERTIES);
	TraceProp.prop.Wnode.ClientContext = 1; // QPC (Default)
	TraceProp.prop.Wnode.Flags = WNODE_FLAG_TRACED_GUID;
	TraceProp.prop.LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
	TraceProp.prop.LogFileNameOffset = offsetof(MY_EVENT_TRACE_PROPERTIES, LogFileName);
	TraceProp.prop.LoggerNameOffset  = offsetof(MY_EVENT_TRACE_PROPERTIES, LoggerName);
	strcat_s(TraceProp.LogFileName, 1024, gLogFilePath);
	strcat_s(TraceProp.LoggerName, 1024, gSessionName);
	
	ErrCode = StartTraceA(
		&hTrace,
		gSessionName,
		&TraceProp
	);
	printf("StartTraceA: 0x%X, hTrace: 0x%X\n", ErrCode, hTrace);

	ErrCode = EnableTraceEx2(
		hTrace,
		&ProviderGuid,
		EVENT_CONTROL_CODE_ENABLE_PROVIDER,
		TRACE_LEVEL_INFORMATION,
		0x0000000000000030, // WINEVENT_KEYWORD_THREAD
		0, 0, NULL
	);
	printf("EnableTraceEx2: 0x%X\n", ErrCode);


	TRACEHANDLE hConsumeTrace;
	EVENT_TRACE_LOGFILEA TraceLogFile = { 0 };
	TraceLogFile.LoggerName = gSessionName;
	TraceLogFile.LogFileName = NULL;
	TraceLogFile.Context = &Pid;
	TraceLogFile.ProcessTraceMode = PROCESS_TRACE_MODE_EVENT_RECORD | PROCESS_TRACE_MODE_REAL_TIME;
	TraceLogFile.EventRecordCallback = (PEVENT_RECORD_CALLBACK)EventRecordCallback;
	TraceLogFile.BufferCallback = (PEVENT_TRACE_BUFFER_CALLBACKA)BufferCallback;

	hConsumeTrace = OpenTraceA(
		&TraceLogFile
	);
	if (hConsumeTrace == INVALID_PROCESSTRACE_HANDLE)
		printf("OpenTraceA failed 0x%X\n", GetLastError());
	printf("hConsumeTrace: 0x%X\n", hConsumeTrace);

	ErrCode = ProcessTrace(
		&hConsumeTrace,
		1,
		0,
		0
	);
	printf("ProcessTrace: 0x%X\n", ErrCode);

	
	ErrCode = CloseTrace(
		hConsumeTrace
	);
	printf("CloseTrace: 0x%X\n", ErrCode);

	ErrCode = StopTraceA(
		hTrace,
		gSessionName,
		&TraceProp
	);
	printf("StopTraceA: 0x%X\n", ErrCode);

	return 0;
}