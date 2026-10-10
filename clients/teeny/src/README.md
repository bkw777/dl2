
# TEENY regenerated

A single common TEENY.S85 asm source and Makefile that generates binaries for all different models of machine.

The 100, M10, and K85 binaries built from this source all work.

The 200 & NEC binaries from this source do not work yet,  
but [../orig/disasm](../orig/disasm) has seperate sources for 200 & NEC,  
and those both work.

So there is working asm source for all machines one way or another.

Error messages are different than the original TINY/TEENY

| ERR | Meaning | Additional |
| --- | --- | --- |
| SN | SyNtax | missing/invalid filename or command |
| FF | File Found | file NOT found, other directory related errors |
| IO | I/O | crc & other rs232 & disk data integrity/sanity errors |
| WP | Write-Protect | also some format errors |
| DF | Disk Full | directory full, max filesize, out of memory |
| ND | No Disk | not inserted, disk changed, drive not ready |
| HW | HardWare | physical drive or disk problems, various sensors |
| UE | Unknown | other/unknown errors |

# references
[TEENY Manual](../teenydoc.txt)

[ROM Cross Reference](https://github.com/bkw777/m100_dev)
