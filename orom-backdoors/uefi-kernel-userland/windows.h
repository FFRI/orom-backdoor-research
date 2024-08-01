/*
 * (c) FFRI Security, Inc., 2024 / Author: FFRI Security, Inc.
 */

#define MSABI __attribute__((__ms_abi__))

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

typedef
void
(MSABI* WINAPI_OUTPUTDEBUGSTRINGA)(
    IN  char*   lpOutputString
    );
WINAPI_OUTPUTDEBUGSTRINGA OutputDebugStringA;
