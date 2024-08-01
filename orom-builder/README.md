# OROM Builder
A simple tool for building an OROM image from an EFI file.  
(EDK2 already contains [EfiRom](https://github.com/tianocore/edk2/blob/master/BaseTools/Source/C/EfiRom/EfiRom.c) for doing the same thing, but this is a minimal implementation and would be good for learning fundamental OROM structures.)  

## Build
```
gcc makeorom.c -o makeorom.exe
```

## Usage
```
.\makeorom.exe in.efi out.bin
```

### How to execute the image on the target machine?
There are 2 methods.
1. Write the image directly to the ROM chip on external devices (this can for example be done by [spi-writer](../orom-flasher)) and boot target with the device
1. Put the image on a USB memory and boot to the UEFI Shell. Then execute `loadpcirom out.bin`.

Though method 2 is simpler, UEFI Shell executes at the BDS phase meaning that the DXE driver in the OROM gets executed late. Normally, OROM gets executed early in the DXE phase (PCI enumeration phase) so some code which works fine using method 2 might not work when put on a real device.
For example, if you use a UEFI protocol which is installed after the PCI enumeration phase (without using `gBS->RegisterProtocolNotify`), method 2 will work, but it fails to find the protocol in method 1.


## Details
What this tool is doing is just adding OROM headers to a EFI file. Since the header contains size field (in units of 512 bytes), the tool calculates the size of the EFI file and modifies the header.  
There are other fields in the header that you might want to customize (e.g., DXE driver's type, architecture, Vendor ID, ...). In that case, you can directly modify the `header` byte array and recompile it. The tool's default settings is:
- Type: EFI Runtime Driver
- Architecture: x86-64
- Compression: Uncompressed
- Number of EFI files: 1 (It is possible to put multiple EFI drivers in a OROM image)


## Reference
- [UEFI spec 2.9 errata C, 14.4.2 PCI Option ROMS](https://uefi.org/sites/default/files/resources/UEFI_Spec_2_8_C_Jan_2021.pdf#page=797)
- [UEFI spec 2.9 errata C, Table 135. Recommended PCI Device Driver Layout](https://uefi.org/sites/default/files/resources/UEFI_Spec_2_8_C_Jan_2021.pdf#page=807)
- [UEFI spec 2.9 errata C, 2.5.1 Legacy Option ROM Issues](https://uefi.org/sites/default/files/resources/UEFI_Spec_2_8_C_Jan_2021.pdf#page=127&zoom=100,96,201)
