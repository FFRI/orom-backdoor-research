/*
 * (c) FFRI Security, Inc., 2024 / Author: FFRI Security, Inc.
 */

#define packed __attribute__((__packed__))

#define MAGIC_MZ       0x5A4D        // 'MZ'

#define BYTE      unsigned char
#define WORD      unsigned short
#define DWORD     unsigned int
#define QWORD     unsigned long
#define ULONG     unsigned long
#define ULONGLONG unsigned long long
#define UINT      unsigned int
#define HANDLE    void*
#define PVOID     void*
#define LONG      long
#define LONGLONG  long long
#define USHORT    unsigned short
#define ULONG64   UINT64

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES 16
#define IMAGE_DIRECTORY_ENTRY_EXPORT 0
#define IMAGE_DIRECTORY_ENTRY_IMPORT 1



typedef struct _IMAGE_EXPORT_DIRECTORY {
  DWORD   Characteristics;
  DWORD   TimeDateStamp;
  DWORD   Version;
  DWORD   Name;
  DWORD   Base;
  DWORD   NumberOfFunctions;
  DWORD   NumberOfNames;
  DWORD   AddressOfFunctions;     // RVA from base of image
  DWORD   AddressOfNames;         // RVA from base of image
  DWORD   AddressOfNameOrdinals;  // RVA from base of image
} packed IMAGE_EXPORT_DIRECTORY, *PIMAGE_EXPORT_DIRECTORY;


typedef struct _IMAGE_IMPORT_BY_NAME {
  WORD  Hint;
  CHAR8 Name[1];
} packed IMAGE_IMPORT_BY_NAME, *PIMAGE_IMPORT_BY_NAME;


typedef struct _IMAGE_THUNK_DATA64 {
  union {
    QWORD                 ForwarderString;
    QWORD                 Function;
    QWORD                 Ordinal;
    PIMAGE_IMPORT_BY_NAME AddressOfData;
  } u1;
} packed IMAGE_THUNK_DATA64, *PIMAGE_THUNK_DATA64;


typedef struct _IMAGE_IMPORT_DESCRIPTOR {
  DWORD OriginalFirstThunk;
  DWORD TimeDateStamp;
  DWORD ForwarderChain;
  DWORD Name;
  DWORD FirstThunk;
} packed IMAGE_IMPORT_DESCRIPTOR, *PIMAGE_IMPORT_DESCRIPTOR;


typedef struct _IMAGE_DATA_DIRECTORY {
  DWORD VirtualAddress;
  DWORD Size;
} packed IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;


typedef struct _IMAGE_OPTIONAL_HEADER64 {
  WORD        Magic;
  BYTE        MajorLinkerVersion;
  BYTE        MinorLinkerVersion;
  DWORD       SizeOfCode;
  DWORD       SizeOfInitializedData;
  DWORD       SizeOfUninitializedData;
  DWORD       AddressOfEntryPoint;
  DWORD       BaseOfCode;
  QWORD       ImageBase;
  DWORD       SectionAlignment;
  DWORD       FileAlignment;
  WORD        MajorOperatingSystemVersion;
  WORD        MinorOperatingSystemVersion;
  WORD        MajorImageVersion;
  WORD        MinorImageVersion;
  WORD        MajorSubsystemVersion;
  WORD        MinorSubsystemVersion;
  DWORD       Win32VersionValue;
  DWORD       SizeOfImage;
  DWORD       SizeOfHeaders;
  DWORD       CheckSum;
  WORD        Subsystem;
  WORD        DllCharacteristics;
  QWORD       SizeOfStackReserve;
  QWORD       SizeOfStackCommit;
  QWORD       SizeOfHeapReserve;
  QWORD       SizeOfHeapCommit;
  DWORD       LoaderFlags;
  DWORD       NumberOfRvaAndSizes;
  IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} packed IMAGE_OPTIONAL_HEADER64, *PIMAGE_OPTIONAL_HEADER64;


typedef struct _IMAGE_NT_HEADERS64 {
  BYTE                    tmp[0x18];
  IMAGE_OPTIONAL_HEADER64 OptionalHeader;
} packed IMAGE_NT_HEADERS64, *PIMAGE_NT_HEADERS64;


typedef struct _IMAGE_DOS_HEADER {
  BYTE  tmp[0x3c];
  DWORD e_lfanew;
} packed IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;




PVOID
GetExport(
    UINT64 ImageBase,
    CHAR8* SymbolName
    )
{
  PIMAGE_DOS_HEADER       dosHeader;
  PIMAGE_NT_HEADERS64     ntHeaders;
  DWORD                   exportRva;
  PIMAGE_EXPORT_DIRECTORY exportDir;

  dosHeader = (PIMAGE_DOS_HEADER)ImageBase;
  ntHeaders = (PIMAGE_NT_HEADERS64)(ImageBase + dosHeader->e_lfanew);
  exportRva = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
  exportDir = (PIMAGE_EXPORT_DIRECTORY)(ImageBase + exportRva);

  DWORD *nameTable     = (DWORD*)(ImageBase + exportDir->AddressOfNames);
  WORD  *ordinalTable  = (WORD*)(ImageBase + exportDir->AddressOfNameOrdinals);
  DWORD *functionTable = (DWORD*)(ImageBase + exportDir->AddressOfFunctions);

  DWORD i;
  for(i=0; i<exportDir->NumberOfNames; i++) {
    char* curSymbolName = (char*)(nameTable[i] + ImageBase);
    if(AsciiStriCmp(curSymbolName, SymbolName) == 0) {
      WORD ordinal = ordinalTable[i];
      return (void*)(ImageBase + functionTable[ordinal]);
    }
  }
  return NULL;
}



PVOID
GetProcAddress(
    UINT64    ImageBase,
    CHAR8*    DllName,
    CHAR8*    FuncName
    )
{
  PIMAGE_DOS_HEADER        dosHeader;
  PIMAGE_NT_HEADERS64      ntHeaders;
  DWORD                    importRva;
  PIMAGE_IMPORT_DESCRIPTOR importDesc;

  dosHeader = (PIMAGE_DOS_HEADER)ImageBase;
  ntHeaders = (PIMAGE_NT_HEADERS64)(ImageBase + dosHeader->e_lfanew);

  importRva = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
  importDesc = (PIMAGE_IMPORT_DESCRIPTOR)(ImageBase + importRva);
  
  for(; importDesc->Name; importDesc++) {
    CHAR8* CurDllName = (CHAR8*)(ImageBase + (UINT64)importDesc->Name);
    if(AsciiStriCmp(DllName, CurDllName) != 0)
      continue;

    PIMAGE_THUNK_DATA64 FirstThunk = (PIMAGE_THUNK_DATA64)(ImageBase + importDesc->FirstThunk);
    PIMAGE_THUNK_DATA64 OriginalFirstThunk = (PIMAGE_THUNK_DATA64)(ImageBase + importDesc->OriginalFirstThunk);
    for(; FirstThunk->u1.Function; FirstThunk++, OriginalFirstThunk++) {
      PIMAGE_IMPORT_BY_NAME ImportByName = (PIMAGE_IMPORT_BY_NAME)(ImageBase + (UINT64)OriginalFirstThunk->u1.AddressOfData);
      if(AsciiStriCmp(FuncName, ImportByName->Name) == 0)
        return (PVOID)FirstThunk->u1.Function;
    }
  }
  return NULL;
}
