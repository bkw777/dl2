
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

# build

default config: 19.2k baud, omit dsr check, include error codes  
`$ make clean all`

include dsr check, larger binary, don't lock up if drive turned off or not connected  
`$ make clean all XFLAGS='-DINCLUDE_DSR'`

omit error codes, all errors just say "ERR", 653 bytes including CO header!  
`$ make clean all XFLAGS='-DNOERRS'`

Makefile build options. Use: `XFLAGS=-DFOO -DBAR=baz ...`  
`-DHIMEM=addr`		default=MAXRAM (of a 32k machine), generate a relocated binary with specified END address  
`-DINCLUDE_DSR`		defualt no, include code for DSR check  
`-DBAUD=9600`		default=19200, 19200 9600 4800 2400 1200 600 300 110 75  
`-DCHUNK_LEN=64`	default=128, 1-128, size of chunks to use saving files  
`-DNOERRS`			omit error codes, all errors just say "ERR"  

Default baud is 19200 because TPDD2 only supports 19200 and cannot be changed,   
And TPDD1 is also set to 19200 by default (all dip switches set to off).

FB-100, FDD19, and Purple Computing drives are all hard-wired for 9600 baud,
but only by a removable solder-blob. For any of these drives you can either
remove the solder-blocb with solder wick, or use `-DBAUD=9600`

Example, build just the Model 100 North America binary with some options changed:
`$ make 100na XFLAGS='-DHIMEM=52000 -DBAUD=9600 -DINCLUDE_DSR -DNOERRS'

# references
[TEENY Manual](../teenydoc.txt)

[ROM Cross Reference](https://github.com/bkw777/m100_dev)
