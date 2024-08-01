# OROM Backdoor Research
While there are few studies inserting malicious code into UEFI Option ROMs (OROMs), none of them have focused soley on OROMs.
In our presentation at [Black Hat USA 2024](https://blackhat.com/us-24/briefings/schedule/#youve-already-been-hacked-what-if-there-is-a-backdoor-in-your-uefi-orom-39579), we organized the benefits and infection scenarios of placing a backdoor in UEFI OROM. This repository contains the PoC code of UEFI OROM backdoors (stripped for security purpose, full source given on demand) and some simple tools that I used in my research.

This repository contains the following contents (details are in the README.md inside each folder).
* orom-builder: A simple tool to convert OROM image from DXE module
* orom-flasher: A sample BusPirate script to write file to the SPI flash chip (OROM)
* orom-backdoors: Source codes of 3 PoC OROM backdoors (stripped)
* EtwConsumer: A simple ETW consumer for tracing only specified process

## Author
Kazuki Matsuo. © FFRI Security, Inc. 2024

## License
Apache version 2.0