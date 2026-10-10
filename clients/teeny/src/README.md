
# TEENY regenerated

A single common TEENY.S85 asm source and Makefile that generates binaries for all different models of machine.

Currently the default config produces a .CO file for Model 100 that is only 646 bytes.

The 100, M10, and K85 binaries built from this source all work.

The 200 & NEC binaries from this source do not work yet,  
but [../orig/disasm](../orig/disasm) has seperate sources for 200 & NEC,  
and those both work.

So there is working asm source for all machines one way or another.

# build

`$ make clean all`

## options
Use: `make ... XFLAGS="-DFOO -DBAR=baz"`

`-DHIMEM=addr`		default=MAXRAM (of a 32k machine), generate a relocated binary with specified END address  
`-DINCLUDE_DSR`		defualt no, include code for DSR check  
`-DBAUD=9600`		default=19200, 19200 9600 4800 2400 1200 600 300 110 75  
`-DCHUNK_LEN=64`	default=128, 1-128, size of chunks to use saving files  
`-DINCLUDE_ERRS`	default no, include code for distinct error codes instead of just "ER" for all (adds over 100 bytes!)  

Default baud is 19200 because TPDD2 only supports 19200 and cannot be changed,   
And TPDD1 is also set to 19200 by default (all dip switches set to off).

FB-100, FDD19, and Purple Computing drives are all hard-wired for 9600 baud.  
For those drives you can build with `-DBAUD=9600`, or you can change them to
19200 by removing the solder blob under the small door on the bottom.

Unlike original TINY/TEENY, by default all errors just say "ER".

`-DINCLUDE_ERRS` adds these distinct error codes  
These are similar to but not identical to legacy TEENY.

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
