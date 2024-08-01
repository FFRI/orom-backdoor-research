/*
 * (c) FFRI Security, Inc., 2024 / Author: FFRI Security, Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define BYTE unsigned char
#define ULONGLONG unsigned long long
#define SZ1_OFFSET 0x02
#define SZ2_OFFSET 0x2C

BYTE header[0x34] = {
  0x55, 0xAA,              // ROM Signature
  0xFF, 0x00,              // *** Initialization Size in units of 512 byte***
  0xF1, 0x0E, 0x00, 0x00,  // EFI Signature
  0x0C, 0x00,              // Subsystem Value (0x0B: EFI Boot Service Driver, 0x0C: EFI Runtime Driver)
  0x64, 0x86,              // Machine Type
  0x00, 0x00,              // *** Compression Type (0:Uncompressed, 1:Compressed) ***
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,    // Reserved
  0x34, 0x00,              // Offset to EFI image
  0x1C, 0x00,              // Offset to PCIR Data Structure
  0x00, 0x00,              // padding to align PCIR on 4 byte boundary
  0x50, 0x43, 0x49, 0x52,  // 'PCIR'
  0x7B, 0x19,              // Vendor ID
  0x85, 0x05,              // Device ID
  0x00, 0x00,              // Reserved
  0x18, 0x00,              // Length of PCIR data struct
  0x00,                    // PCIR data struct revision
  0x01, 0x06, 0x01,        // Class Code from the PCI Controller's Configuration Header
  0xFF, 0x00,              // *** Code Image Length in units of 512 byte (Same as Initialization Size) ***
  0x00, 0x00,              // Revision Level of the ... (this field is ignored)
  0x03,                    // Code Type
  0x80,                    // *** Indicator. Bit7=1 this is the last image in PCI OROM. Bit7=0 there are another image follows. ***
  0x00, 0x00,              // Reserved (ここまでで0x34バイト)
};

void print(void* buf, int offset, int count) {
	int i, j;
	for(i=0; i<count; i+=4) {
		for(j=0; j<4; j++) {
			printf("%02X ", *(BYTE*)(buf+offset+i+j));
		}
		printf("\n");
	}
}

int main(int argc, char **argv) {
	if(argc!=3) {
		puts("usage: makeorom.exe <in.efi> <out.bin>");
		return 1;
	}

	// 1: get efi file size and ptr
	FILE *fileptr;
	ULONGLONG filelen = 0;
	fileptr = fopen(argv[1], "rb");
	fseek(fileptr, 0ull, SEEK_END);
	filelen = ftell(fileptr);
	printf("file length: 0x%X\n", filelen);
	rewind(fileptr);

	// 2: merge header and file contents to data
	BYTE *data;
	ULONGLONG sz = filelen + sizeof(header);

	data = (BYTE*)malloc(sz * sizeof(BYTE));

	memcpy(data, header, sizeof(header));
	fread(data+sizeof(header), filelen, 1, fileptr);
	fclose(fileptr);

	// 3: adjust the header size
	ULONGLONG pagesz = sz/512 + 1;
	if(pagesz%2!=0) pagesz+=1;
	*(uint16_t*)(data+0x02) = pagesz;
	*(uint16_t*)(data+0x2C) = pagesz;

	// (check)
	int i;
	puts("data[0]-data[0x10]:");
	print(data, 0x00, 0x34);
	puts("\ndata[0x34]-data[0x44]:");
	print(data, 0x34, 0x10);

	// 4: output to file
	char c;
	printf("write this to file? [y/n]: ");
	c = getc(stdin);
	if(c!='y')
		goto END;

	FILE *f = fopen(argv[2], "wb");
	fwrite(data, sz, 1, f);
	fclose(f);

END:
	free(data);
	return 0;
}
