# OROM Malwares
3 PoC UEFI OROM backdoors.  
From a security standpoint, only the essential parts (those with novelty) of the code are disclosed here.
**If you require the full source code, please contact us via email (research-feedback@ffri.jp). Once your identity is verified, we will send it to you directly.**
For each of the three PoCs, only the following are published on GitHub, while the rest are omitted:
* UEFI
  * Use of HttpProtocol via ExitBootServices hook
* UEFI+Kernel+Userland
  * Allocation of shellcode buffer during runtime
  * Modification of Supervisor bit
  * Part of the code for CFG bypass related to paging
  * Hook of SetVariable
  * SetVirtualAddressMap event handler

Since each technique used by the UEFI+Kernel backdoor is not novel, all the source code of this backdoor is not disclosed.
However, it's essential to note that when using kernel exports from the runtime DXE modules,
considerations such as the inability to use some functions like IoSetCompletionRoutine need to be taken into account.

