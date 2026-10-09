
# [ROM Cross Reference](https://github.com/bkw777/m100_dev)

This is a single common TEENY.S85 with only a different header file for each model of machine.

The 100, M10, and K85 binaries built from this source all work.

The 200 & NEC binaries from this source do not work,  
but [../orig/disasm](../orig/disasm) has seperate sources just for 200 & NEC,  
and those both work.

So there is working asm source for all machines one way or another.

Error messages are different than the original TINY/TEENY  
FF - File (not)Found - directory related errors  
UE - Unknown Error  
SN - SyNtax errors  
IO - I/O - crc & other rs232 & disk data integrity/sanity checks  
WP - Write-Protect, format  
DF - Dir/Disk Full, max filesize exceeded, out of memory  
ND - No Disk - disk changed / not inserted  
HW - HardWare - physical drive or disk problems
