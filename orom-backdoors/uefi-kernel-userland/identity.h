/*
 * (c) FFRI Security, Inc., 2024 / Author: FFRI Security, Inc.
 */

UINT64 gIdentityPml40;



VOID
MakeIdentityPageTable (
    )
{
  // PML4 size = 8 Bytes (PML4E size) * 512 PML4Es = 4096(1 page size)
  //  Use AllocatePages to 4KB align the address
  EFI_PHYSICAL_ADDRESS pml4;   // pml4[512]
  gBS->AllocatePages(
      AllocateAnyPages,
      EfiRuntimeServicesCode,
      1,
      &pml4
      );

  EFI_PHYSICAL_ADDRESS pdp;   // pdp[512]
  gBS->AllocatePages(
      AllocateAnyPages,
      EfiRuntimeServicesCode,
      1,
      &pdp
      );

  // 1 PD contains 512 2MB pages (= can describe 1GB of phyical address)
  // => we need 64 PD
  EFI_PHYSICAL_ADDRESS pd;   // pd[64][512]
  gBS->AllocatePages(
      AllocateAnyPages,
      EfiRuntimeServicesCode,
      64,
      &pd
      );

  // link page tables
  *(UINT64*)pml4 = (UINT64)pdp | 0x003;
  for(UINT64 i=0; i<64; i++) {
    *(UINT64*)(pdp+i*8) = ((UINT64)pd + i*8*512) | 0x003;
    for(UINT64 j=0; j<512; j++)
      *(UINT64*)(pd + i*8*512 + j*8) = (i*(UINT64)SIZE_1GB + j*(UINT64)SIZE_2MB) | 0x083;
  }

  // save the (physical) address of PML4[0]
  gIdentityPml40 = (UINT64)pml4;
}





VOID
PhysicalWrite(
    UINT64 VirtualAddress,
    BYTE*  Data,
    UINTN  DataSize
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
  

  VIRTUAL_ADDR_4KB vaddr;
  vaddr.value = VirtualAddress;

  IA32_PML4E *pml4e;
  IA32_PDPE  *pdpe;
  IA32_PDE   *pde;
  IA32_PTE   *pte;

  UINTN phy_base;
  BYTE* phy_addr;

  __asm__ __volatile__("invlpg %0" :: "m"(Pml4BasePaddr));
  pml4e = (IA32_PML4E*)(Pml4BasePaddr + 8*vaddr.PML4index);

  UINTN pdp_base = pml4e->value & mask;
  __asm__ __volatile__("invlpg %0" :: "m"(pdp_base));
  pdpe = (IA32_PDPE*)(pdp_base + 8*vaddr.PDPindex);

  if(pdpe->PageSize) {  // 1GB page
    VIRTUAL_ADDR_1GB vaddr_1gb;
    vaddr_1gb.value = VirtualAddress;
    mask &= ~((UINTN)0x3FFFFFFF);
    phy_base = pdpe->value & mask;
    phy_addr = (BYTE*)(phy_base + vaddr_1gb.Offset);
    goto CopyData;
  }

  UINTN pd_base = pdpe->value & mask;
  __asm__ __volatile__("invlpg %0" :: "m"(pd_base));
  pde = (IA32_PDE*)(pd_base + 8*vaddr.PDindex);

  if(pde->PageSize) {  // 2MB page
    VIRTUAL_ADDR_2MB vaddr_2mb;
    vaddr_2mb.value = VirtualAddress;
    mask &= ~((UINTN)0x1FFFFF);
    phy_base = pde->value & mask;
    phy_addr = (BYTE*)(phy_base + vaddr_2mb.Offset);
    goto CopyData;
  }

  // 4KB page
  UINTN pt_base = pde->value & mask;
  __asm__ __volatile__("invlpg %0" :: "m"(pt_base));
  pte = (IA32_PTE*)(pt_base + 8*vaddr.PTindex);
  
  phy_base = pte->value & mask;
  __asm__ __volatile__("invlpg %0" :: "m"(phy_base));
  phy_addr = (BYTE*)(phy_base + vaddr.Offset);

CopyData:
  UINTN i;
  for(i=0; i<DataSize; i++)
    phy_addr[i] = Data[i];

  // Switch back paging
  CopyMem(Pml4BaseVaddr, &OrigPml40, 8);
}
