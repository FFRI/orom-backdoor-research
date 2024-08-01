/*
 * (c) FFRI Security, Inc., 2024 / Author: FFRI Security, Inc.
 */

#define NTSTATUS    long
#define UCHAR       unsigned char
#define PLIST_ENTRY LIST_ENTRY*


typedef struct _UNICODE_STRING {
  USHORT  Length;
  USHORT  MaxLength;
  CHAR16* Buffer;
} UNICODE_STRING, *PUNICODE_STRING;


typedef union _LARGE_INTEGER {
    struct {
        ULONG LowPart;
        LONG HighPart;
    } DUMMYSTRUCTNAME;
    struct {
        ULONG LowPart;
        LONG HighPart;
    } u;
    LONGLONG QuadPart;
} LARGE_INTEGER;


typedef struct _CLIENT_ID {
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID, *PCLIENT_ID;


typedef struct _OBJECT_ATTRIBUTES {
  ULONG           Length;
  HANDLE          RootDirectory;
  PUNICODE_STRING ObjectName;
  ULONG           Attributes;
  PVOID           SecurityDescriptor;
  PVOID           SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;


typedef struct _EX_FAST_REF {
   UINT64 Value;
} EX_FAST_REF;

typedef struct _PEB_LDR_DATA {
  char tmp[0x20];                     // offset size
  LIST_ENTRY InMemoryOrderModuleList; // 0x20   0x10
} __attribute__((__packed__)) PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _PEB {
  char tmp[0x3];                    // offset size
  struct {
    UINT8 tmp                : 1;
    UINT8 IsProtectedProcess : 1;
    UINT8 tmp2               : 6;
  } BitField;                       // 0x3    0x1
  char tmp2[0xc];                   // 0x4    0xc
  UINT64 ImageBaseAddress;          // 0x10   0x8
  PPEB_LDR_DATA Ldr;                // 0x18   0x8
  char tmp3[0x388];                 // 0x20   0x388
} __attribute__((__packed__)) PEB, *PPEB;

typedef struct _EPROCESS {
   char         tmp[0x28];           // offset size
   UINT64       DirectoryTableBase;  // 0x28   0x8
   char         tmp1[0x410];         // 0x30   0x410
   ULONG*       UniqueProcessId;     // 0x440  0x8
   LIST_ENTRY   ActiveProcessLinks;  // 0x448  0x10
   char         tmp2[0x60];          // 0x458  0x60
   EX_FAST_REF  Token;               // 0x4b8  0x8
   char         tmp3[0x90];          // 0x4c0  0x90
   PEB*         peb;                 // 0x550  0x8
   char         tmp4[0x50];          // 0x558  0x50
   UCHAR        ImageFileName[15];   // 0x5a8  0x15
   char         tmp5[0x473];         // 0x5bd  0x473
} __attribute__((__packed__)) EPROCESS, *PEPROCESS;


typedef struct _KAPC {                  
    UCHAR Type;                         
    UCHAR SpareByte0;                   
    UCHAR Size;                         
    UCHAR SpareByte1;                   
    ULONG SpareLong0;                   
    PVOID Thread;            
    LIST_ENTRY ApcListEntry;            
    PVOID Reserved[3];                  
    PVOID NormalContext;                
    PVOID SystemArgument1;              
    PVOID SystemArgument2;              
    char  ApcStateIndex;                
    char  ApcMode;            
    BOOLEAN Inserted;                   
} KAPC, *PKAPC, *PRKAPC;    


typedef struct _M128A {
    ULONGLONG Low;
    LONGLONG High;
} __attribute__((aligned (16))) M128A, *PM128A;


typedef struct _CONTEXT {
    //
    // Register parameter home addresses.
    //
    // N.B. These fields are for convience - they could be used to extend the
    //      context record in the future.
    //
    ULONG64 P1Home;
    ULONG64 P2Home;
    ULONG64 P3Home;
    ULONG64 P4Home;
    ULONG64 P5Home;
    ULONG64 P6Home;

    //
    // Control flags.
    //
    ULONG ContextFlags;
    ULONG MxCsr;

    //
    // Segment Registers and processor flags.
    //
    USHORT SegCs;
    USHORT SegDs;
    USHORT SegEs;
    USHORT SegFs;
    USHORT SegGs;
    USHORT SegSs;
    ULONG EFlags;

    //
    // Debug registers
    //
    ULONG64 Dr0;
    ULONG64 Dr1;
    ULONG64 Dr2;
    ULONG64 Dr3;
    ULONG64 Dr6;
    ULONG64 Dr7;

    //
    // Integer registers.
    //
    ULONG64 Rax;
    ULONG64 Rcx;
    ULONG64 Rdx;
    ULONG64 Rbx;
    ULONG64 Rsp;
    ULONG64 Rbp;
    ULONG64 Rsi;
    ULONG64 Rdi;
    ULONG64 R8;
    ULONG64 R9;
    ULONG64 R10;
    ULONG64 R11;
    ULONG64 R12;
    ULONG64 R13;
    ULONG64 R14;
    ULONG64 R15;

    //
    // Program counter.
    //
    ULONG64 Rip;

    //
    // Floating point state.
    //
    union {
        //XMM_SAVE_AREA32 FltSave;
        struct {
            M128A Header[2];
            M128A Legacy[8];
            M128A Xmm0;
            M128A Xmm1;
            M128A Xmm2;
            M128A Xmm3;
            M128A Xmm4;
            M128A Xmm5;
            M128A Xmm6;
            M128A Xmm7;
            M128A Xmm8;
            M128A Xmm9;
            M128A Xmm10;
            M128A Xmm11;
            M128A Xmm12;
            M128A Xmm13;
            M128A Xmm14;
            M128A Xmm15;
        } DUMMYSTRUCTNAME;
    } DUMMYUNIONNAME;

    //
    // Vector registers.
    //
    M128A VectorRegister[26];
    ULONG64 VectorControl;

    //
    // Special debug control registers.
    //
    ULONG64 DebugControl;
    ULONG64 LastBranchToRip;
    ULONG64 LastBranchFromRip;
    ULONG64 LastExceptionToRip;
    ULONG64 LastExceptionFromRip;
} __attribute__((aligned (16))) CONTEXT, *PCONTEXT;
