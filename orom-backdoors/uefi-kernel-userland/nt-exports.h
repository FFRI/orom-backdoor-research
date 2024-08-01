/*
 * (c) FFRI Security, Inc., 2024 / Author: FFRI Security, Inc.
 */

#define NTAPI __attribute__((__ms_abi__))
#define MSR_IA32_LSTAR 0xC0000082


PEPROCESS PsInitialSystemProcess;

typedef
NTSTATUS
(NTAPI *ZWWAITFORSINGLEOBJECT)(
    HANDLE     Handle,
    BOOLEAN    Alertable,
    LONGLONG*  Timeout
    );
ZWWAITFORSINGLEOBJECT ZwWaitForSingleObject;

typedef
NTSTATUS
(NTAPI *NTSETINFORMATIONPROCESS)(
    IN  HANDLE  ProcessHandle,
    IN  char    ProcessInformationClass,
    IN  PVOID   ProcessInformation,
    IN  ULONG   ProcessInformationLength
    );
NTSETINFORMATIONPROCESS NtSetInformationProcess;

typedef
VOID
(NTAPI *KEENTERGUARDEDREGION)(
    );
KEENTERGUARDEDREGION KeEnterGuardedRegion;

typedef
VOID
(NTAPI *KELEAVEGUARDEDREGION)(
    );
KELEAVEGUARDEDREGION KeLeaveGuardedRegion;

typedef
VOID
(NTAPI *KESTACKATTACHPROCESS)(
    IN   PEPROCESS proc,
    OUT  PVOID     ApcState
    );
KESTACKATTACHPROCESS KeStackAttachProcess;

typedef
VOID
(NTAPI *KEUNSTACKDETACHPROCESS)(
    IN   PVOID     ApcState
    );
KEUNSTACKDETACHPROCESS KeUnstackDetachProcess;

typedef
PEPROCESS
(NTAPI *PSGETCURRENTPROCESS)(
    );
PSGETCURRENTPROCESS PsGetCurrentProcess;

typedef
HANDLE
(NTAPI *PSGETPROCESSID)(
    PEPROCESS  Process
    );
PSGETPROCESSID PsGetProcessId;

typedef
NTSTATUS
(NTAPI *ZWOPENPROCESS)(
    HANDLE*             ProcessHandle,
    ULONG               DesiredAccess,
    POBJECT_ATTRIBUTES  ObjectAttributes,
		PCLIENT_ID          ClientId
    );
ZWOPENPROCESS ZwOpenProcess;

typedef
NTSTATUS
(NTAPI *SELOCATEPROCESSIMAGENAME)(
    IN   PEPROCESS        Process,
    OUT  PUNICODE_STRING  *pImageFileName
    );
SELOCATEPROCESSIMAGENAME SeLocateProcessImageName;

typedef
NTSTATUS
(NTAPI *ZWPROTECTVIRTUALMEMORY)(
    HANDLE ProcessHandle,
    VOID** BaseAddress,
    ULONG* ProtectSize,
    ULONG  NewProtect,
    ULONG* OldProtect
    );
ZWPROTECTVIRTUALMEMORY ZwProtectVirtualMemory;

typedef
NTSTATUS
(NTAPI *OBOPENOBJECTBYPOINTER)(
    PVOID  Object,
    ULONG  HandleAttributes,
    PVOID  PassedAccessState,
    ULONG  DesiredAccess,
    PVOID  ObjectType,
    CHAR8  AccessMode,
    PVOID  Handle
    );
OBOPENOBJECTBYPOINTER ObOpenObjectByPointer;

typedef
PVOID
(NTAPI *MMGETVIRTUALFORPHYSICAL)(
    UINT64 PhysicalAddress
    );
MMGETVIRTUALFORPHYSICAL MmGetVirtualForPhysical;

typedef
NTSTATUS
(NTAPI *RTLCREATEUSERTHREAD)(
  IN      HANDLE                ProcessHandle,
  IN      PVOID                 SecurityDescriptor,
  IN      BOOLEAN               CreateSuspended,
  IN      ULONG                 StackZeroBits,
  IN OUT  PVOID                 StackReserved,
  IN OUT  PVOID                 StackCommit,
  IN      PVOID                 StartAddress,
  IN      PVOID                 StartParameter,
  OUT     HANDLE*               ThreadHandle,
  OUT     PCLIENT_ID            ClientID
  );
RTLCREATEUSERTHREAD RtlCreateUserThread;





QWORD
GetKernelBase(
    )
{
  // Omitted
}



VOID
ResolveNtosExport(
    )
{
  PsInitialSystemProcess = *(EPROCESS**)GetExport(
      GetKernelBase(),
      "PsInitialSystemProcess"
      );
  SeLocateProcessImageName = (SELOCATEPROCESSIMAGENAME)GetExport(
      GetKernelBase(),
      "SeLocateProcessImageName"
      );
  PsGetCurrentProcess = (PSGETCURRENTPROCESS)GetExport(
      GetKernelBase(),
      "PsGetCurrentProcess"
      );
  ObOpenObjectByPointer = (OBOPENOBJECTBYPOINTER)GetExport(
      GetKernelBase(),
      "ObOpenObjectByPointer"
      );
  ZwProtectVirtualMemory = (ZWPROTECTVIRTUALMEMORY)GetExport(
      GetKernelBase(),
      "ZwProtectVirtualMemory"
      );
  KeStackAttachProcess = (KESTACKATTACHPROCESS)GetExport(
      GetKernelBase(),
      "KeStackAttachProcess"
      );
  KeUnstackDetachProcess = (KEUNSTACKDETACHPROCESS)GetExport(
      GetKernelBase(),
      "KeUnstackDetachProcess"
      );
  KeEnterGuardedRegion = (KEENTERGUARDEDREGION)GetExport(
      GetKernelBase(),
      "KeEnterGuardedRegion"
      );
  KeLeaveGuardedRegion = (KELEAVEGUARDEDREGION)GetExport(
      GetKernelBase(),
      "KeLeaveGuardedRegion"
      );
  RtlCreateUserThread = (RTLCREATEUSERTHREAD)GetExport(
      GetKernelBase(),
      "RtlCreateUserThread"
      );
  PsGetProcessId = (PSGETPROCESSID)GetExport(
      GetKernelBase(),
      "PsGetProcessId"
      );
  ZwOpenProcess = (ZWOPENPROCESS)GetExport(
      GetKernelBase(),
      "ZwOpenProcess"
      );
  NtSetInformationProcess = (NTSETINFORMATIONPROCESS)GetExport(
      GetKernelBase(),
      "NtSetInformationProcess"
      );
  MmGetVirtualForPhysical = (MMGETVIRTUALFORPHYSICAL)GetExport(
      GetKernelBase(),
      "MmGetVirtualForPhysical"
      );
  ZwWaitForSingleObject = (ZWWAITFORSINGLEOBJECT)GetExport(
      GetKernelBase(),
      "ZwWaitForSingleObject"
      );
}

