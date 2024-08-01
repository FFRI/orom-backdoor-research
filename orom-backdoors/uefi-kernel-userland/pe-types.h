/*
 * (c) FFRI Security, Inc., 2024 / Author: FFRI Security, Inc.
 */

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
} __attribute__((__packed__)) IMAGE_EXPORT_DIRECTORY, *PIMAGE_EXPORT_DIRECTORY;

typedef struct _IMAGE_IMPORT_BY_NAME {
  WORD  Hint;
  CHAR8 Name[1];
} __attribute__((__packed__)) IMAGE_IMPORT_BY_NAME, *PIMAGE_IMPORT_BY_NAME;

typedef struct _IMAGE_THUNK_DATA64 {
  union {
    QWORD                 ForwarderString;
    QWORD                 Function;
    QWORD                 Ordinal;
    PIMAGE_IMPORT_BY_NAME AddressOfData;
  } u1;
} __attribute__((__packed__)) IMAGE_THUNK_DATA64, *PIMAGE_THUNK_DATA64;

typedef struct _IMAGE_IMPORT_DESCRIPTOR {
  DWORD OriginalFirstThunk;
  DWORD TimeDateStamp;
  DWORD ForwarderChain;
  DWORD Name;
  DWORD FirstThunk;
} __attribute__((__packed__)) IMAGE_IMPORT_DESCRIPTOR, *PIMAGE_IMPORT_DESCRIPTOR;


typedef struct _IMAGE_DATA_DIRECTORY {
  DWORD VirtualAddress;
  DWORD Size;
} __attribute__((__packed__)) IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;


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
} __attribute__((__packed__)) IMAGE_OPTIONAL_HEADER64, *PIMAGE_OPTIONAL_HEADER64;


typedef struct _IMAGE_NT_HEADERS64 {
  BYTE                    tmp[0x18];
  IMAGE_OPTIONAL_HEADER64 OptionalHeader;
} __attribute__((__packed__)) IMAGE_NT_HEADERS64, *PIMAGE_NT_HEADERS64;


typedef struct _IMAGE_DOS_HEADER {
  BYTE  tmp[0x3c];
  DWORD e_lfanew;
} __attribute__((__packed__)) IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;
