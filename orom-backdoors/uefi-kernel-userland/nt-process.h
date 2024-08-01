/*
 * (c) FFRI Security, Inc., 2024 / Author: FFRI Security, Inc.
 */

PEPROCESS
GetProcessByName(
    CHAR16* ProcessName
    )
{
  PEPROCESS proc = PsInitialSystemProcess;
  QWORD offset = (QWORD)&proc->ActiveProcessLinks - (QWORD)proc;
  while(1) {
    PLIST_ENTRY NextActiveProcessLinks = proc->ActiveProcessLinks.ForwardLink;
    proc = (PEPROCESS)( (QWORD)NextActiveProcessLinks - offset );
    PUNICODE_STRING ProcName16;
    SeLocateProcessImageName(proc, &ProcName16);
    if(StrStr(ProcName16->Buffer, ProcessName)!=NULL) {
      return proc;
    }
  }
  return NULL;
}



UINTN gOrigCR3 = 0;
BOOLEAN gFirstTime = TRUE;

VOID
SetProcessContext(
    PEPROCESS targetProc,
    BOOLEAN bSetBack
    )
{
  if(gFirstTime) {
    __asm__ __volatile__("mov %%cr3, %%rax" : "=a"(gOrigCR3));
    gFirstTime = FALSE;
  }

  PEPROCESS curProc = PsGetCurrentProcess();

  if(bSetBack) { // Set CR3 to original value
    curProc->DirectoryTableBase = gOrigCR3;
    __asm__ __volatile__("mov %%rax, %%cr3" : : "a"(gOrigCR3));
  }
  else { // Set DirBase CR3
    UINT64 ProcCr3 = targetProc->DirectoryTableBase;
    curProc->DirectoryTableBase = ProcCr3;
    __asm__ __volatile__("mov %%rax, %%cr3" : : "a"(ProcCr3));
  }
}



UINT64
GetExeBase(
    PEPROCESS proc
    )
{
  PPEB peb = proc->peb;
  return peb->ImageBaseAddress;
}



UINT64
GetLoadedDllBase(
    PEPROCESS proc,
    CHAR16*   DllName
    )
{
  PPEB peb = proc->peb;
  UINT64 DllBaseOffset = 0x20;
  UINT64 BaseDllNameOffset = 0x48 + 0x8;
  UINT64 limit = 20; // search dll limit

  LIST_ENTRY* le = peb->Ldr->InMemoryOrderModuleList.ForwardLink;
  while(limit>0) {
    le = le->ForwardLink;
    CHAR16* BaseDllName = *(CHAR16**)((void*)&le->ForwardLink + BaseDllNameOffset);
    if(StriStri(BaseDllName, DllName)!=NULL)
      return *(UINT64*)((void*)&le->ForwardLink + DllBaseOffset);
    limit--;
  }
  return 0;
}



