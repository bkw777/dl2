
# TEENY regenerated

A single common asm source that aims to eventually generate [TEENY](../teenydoc.txt) binaries for all computers sharing the Kyotronic KC-85 platform.  
(Kyotronic KC-85, TRS-80 Model 100, TANDY 102 200, NEC PC-8201 PC-8201A PC-8300, Olivetti M-10)

Currently the default config produces a .CO file for TRS-80 Model 100 that is only 630 bytes.  
(Legacy TEENY for 100 is 753 bytes)

The 100, M10, and K85 binaries built from this source all work.

The 200 & NEC binaries from this source do not work yet,  
but [../orig/disasm](../orig/disasm) has source for 200 & NEC which both work.

So there is working asm source for at least the North American version of all machines one way or another.

# build

`$ make clean all`

## options

| option=default    | description |
| ---               | --- |
| `BAUD=19200`      | 19200 9600 4800 2400 1200 600 300 110 75 |
| `DSR=false`       | include code for DSR check |
| `ERRCODES=false`  | distinct error codes |
| `HIMEM=<MAXRAM>`  | TOP/EXE = specified HIMEM minus LENGTH |
| `ORG=<HIMEM-LEN>` | TOP/EXE = specified ORG (overrides HIMEM) |
| `ROM=false`       | don't include any self-modifying code |

Use by just adding to the `make` commandline  
`make clean all DSR=true BAUD=9600`

* Default generates a `.CO` file that ends just below MAXRAM on a 32k machine  
* `HIMEM=addr` generates a `.CO` file that ends just below `addr`  
* `ROM=true ORG=addr` generates a `.bin` file that starts at `addr` (overrides HIMEM), and doesn't include any code that modifies the binary at run-time.

The only operational difference from the original TINY/TEENY is that by default all errors print the same "ER" message instead of different error codes for different errors.

`ERRCODES=true` adds these distinct error codes  
These are similar to but not identical to legacy TEENY.

| ERR | Meaning       | Additional |
| --- | ---           | --- |
| SN  | SyNtax        | missing/invalid filename or command |
| FF  | File Found    | dest file already exists, src file not found, other dir errs |
| IO  | I/O           | crc & other rs232 & disk data integrity/sanity errors |
| WP  | Write-Protect | also some format errors |
| DF  | Disk Full     | directory full, max filesize, out of memory |
| ND  | No Disk       | not inserted, disk changed, drive not ready |
| HW  | HardWare      | physical drive or disk problems, various sensors |
| UE  | Unknown       | other/unknown errors |

DSR is not enabled by default to allow the most possible serial cables to work when connecting to a computer instead of a drive.

You will need `BAUD=9600` if you have a Brother FB-100 or knitking FDD19, and don't want to remove the solder-blob those drives have in place of the dip switch.

You will need `ROM=true` if you want to embed TEENY into a rom. This disables the use of some slightly space-saving tricks that rely on being able to modify values in the binary itself at run-time.

# references
[TEENY Manual](../teenydoc.txt)

[ROM Cross Reference](https://github.com/bkw777/m100_dev)
