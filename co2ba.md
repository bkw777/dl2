# co2ba.sh

![co2ba.sh](co2ba.sh)  
Reads a binary .CO file and generates an ascii BASIC loader .DO file

## Usage
`co2ba FILE.CO [action] > FILE.DO`

**FILE.CO** is the input binary .CO filename that you want to bootstrap onto the portable.

**action** is what the loader should do with the .CO after it's done re-creating it on the portable:  
```
 call   - Immediately execute - for TANDY, Kyotronic, Olivetti
 exec   - Immediately execute - for NEC
 callba - Create a launcher NAME.BA - for TANDY, Kyotronic, Olivetti
 execba - Create a launcher NAME.BA - for NEC
 savem  - Save NAME.CO - for TANDY, Kyotronic, Olivetti
 bsave  - Save NAME.CO - for NEC
 time   - Just show how many seconds it took the loader to run
 ```
 Otherwise if the option is not given, or any other value than these, the loader will only print a message showing the Top, End, and Exec addresses of the loaded binary.  

<!--
**comment** is an optional custom replacement text for the first half of the line #0 comment.  
 By default a basic comment giving the name of the .CO file is generated.  
 The 2nd half of the line always has co2ba.sh itself and the date it was run to generate the loader.  
 You can use this to give more info about the payload than just the filename.  
-->

**FILE.DO** is the output ascii BASIC .DO filename.

## Options
A few parameters are run-time configurable by setting environment variables.  
You don't need to change any of these. They exist and are documented here just for flexability and completeness.  
Available settings and their default values:  
```
PN=""          # program/payload name, derived from the input filename by default
COMMENT=""     # extra text inserted into the first line comment
FIRST=0        # first line number
STEP=1         # line number increment
LLEN=256       # length of DATA lines
METHOD=Y       # which encoding scheme: Y=!yenc H=hexpairs I=ints
UNSAFE="0 1 2 3 4 5 6 7 8 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 34"
               # list of byte values that need to be encoded
EDITSAFE=true  # add 127 to the unsafe list so the output can be opened in EDIT
EP='!'         # escape prefix - character that indicates the next byte is encoded
XA=^64         # initial transform applied to all bytes - best,0,^###,+###
XB=^128        # encoding transform applied to unsafe bytes - ^###,+###
RLE=false      # enable run-length encoding (doesn't help, and the loader is much slower)
RP=' '         # rle prefix - character that indicates the next byte is how many copies of the previous byte to append here
CK=xor         # checksum algorithm - xor xor+ mod+ sum+
YENC=false     # output standard yEnc, shorthand for EP='=' XA='+42' XB='+64'
```
<!-- does not work
   "?OD error in 1"
CARAT=false    # output standard carat encoding, shorthand for EP='^' XA=0 XB="+64"
-->

## Encoding schemes
### Method Y  
"!yenc" (default)  
The main idea of this one comes from [Stephen Adolph](https://www.mail-archive.com/m100@lists.bitchin100.com/msg06918.html), modified by [HackerB9 & Brian White](https://www.mail-archive.com/m100@lists.bitchin100.com/msg20099.html).  
It is very similar to [yEnc](http://www.yenc.org/yenc-draft.1.3.txt).  
  - First, apply a simple transform the same way to all input bytes.  
    yenc does rot42 `(val+42)%256`, we do xor64 `val^64`.  
  - Then, for each (transformed) byte:  
    - If a byte is safe, copy it to output without any changes.  
    - If a byte is unsafe,  
      - Output an escape character prefix.  
        yenc uses `=`, we use `!`  
      - Apply another simple transform to make a safe byte.  
        yenc does rot64 `(val+64)%256`, we do xor128 `val^128`, aka toggle the high bit.
<!--
### Method B:  
  Identical data to Y.  
  The difference is the inner loop in BASIC to decode bytes does not use any IF branching.  
  It's actually slower so don't use it.  
  It's just here because if it wasn't, you or I would try to do it again.  
  Also it was hard to figure out so I want to keep it as a reference for tricks. And who knows maybe it can get better.
-->

### Method H:  
  Hex pairs a-la James Yi / Kurt McCullum / others.  
  Hex pairs, but using a single contiguous range of ascii values like a-p for the alphabet instead of 0-9A-F.  
  0x00 = aa, 0x01 = ab, ... 0xFF = pp  
  Some old loaders use this because the code is small and simple, and the output is at least better than plain ints.  
  A lot of old loaders actually used regular hex pairs with the normal 0-9A-F alphabet,
  but that requires larger and slower BASIC code to decode for no benefit other than maybe human readability.

### Method I:  
  Plain comma seperated integers.  
  The simplest possible way to put binary into DATA statements and read them.  
  It's useful for very small payloads because the BASIC to load it is almost nothing.  
  An old example: [TINYLD.DO](https://www.club100.org/library/ups/tinyld.ba)


## Optimization

### XA=n
For !yenc, you can get a slightly smaller output file by using `XA=best` .  
This will internally try all possible XA values with both xor & rot (^0-^255 +0-+255) and pick the best.  
This will take several seconds and generally produce a smaller file, but usually not by much,  
and the loader may actually run slower if "best" lands on a + value rather than a ^ value,  
which is why "best" is not the default.  
Note, every input file will have it's own different best value.

### Run-Length Encoding
`RLE=true` enables run-length-encoding compression.

For most input files this will only make the output larger, and the loader slower.

The encoding scheme is `DRN` , where:  
`D` is a byte of data that is the first byte of a run of duplicates.  
`R` is the rle-prefix character `RP` defined above, default is 0x20, aka space.  
`N` is a single byte value for the number of additional copies of `D` to generate.  

`D` and `N` are encoded as necessary like all other normal data. The !yenc encoding must be decoded first to get the actual value of them.  
`R` (RP above) is NOT encoded. When RLE is enabled, all of the ' ' in the input data get encoded, and ' ' becomes part of the encoding scheme itself like '!'.  
`N` can only encode up to 255. Longer runs simply use multiple rle codes up to 255 each.  
`N` is one less than the length of the total run. `D` is the first byte in the run, `N` adds to it.  

### Checksum
By default all methods include a rolling xor checksum to catch corrupt serial transfers.

There are a few other checksum strategies to choose from if you want something more bulletproof than the simple rolling xor.

xor  - `sum = sum ^ byte`  fastest, only needs INT variables, but will miss some errors in some input data patterns  
xor+ - `sum = sum ^ byte + 1`  slower but stronger (nuls count), and needs DEFSNG variables  
mod+ - `sum=(sum+1+byte)%32512`  strong while still only needing INT variables, but slower  
sum+ - `sum = sum + byte + 1`  strong, needs DEFSNG variables, and suprizingly the slowest

## Examples

[RAM100.DO](https://github.com/bkw777/NODE_DATAPAC/tree/main/software/RAMDSK/RAM100)  
`co2ba RAM100.CO savem >RAM100.DO`

[ALTERN.DO](https://github.com/LivingM100SIG/Living_M100SIG/blob/main/M100SIG/Lib-07-UTILITIES/ALTERN.100)  
`FIRST=50 STEP=5 LLEN=74 EDITSAFE=true co2ba ALTERN.CO call >ALTERN.DO`

## Results

Sample input .CO file = 3620 bytes  
```
$ ls -l ALTERN.CO
-rw-rw-r-- 1 bkw bkw 3620 Feb 28 14:10 ALTERN.CO
```

(All of these actually consume 60 bytes less RAM than the file size, because the first line 0 gets overwritten on the receiving machine.)

Simple INT encoding -> 12478 bytes    
```
$ METHOD=I co2ba ALTERN.CO call >ALTERN.DO ;ls -l ALTERN.DO ;tr '\r' '\n' <ALTERN.DO      
-rw-rw-r-- 1 bkw bkw 12478 Mar  9 15:29 ALTERN.DO
0'ALTERN - loader: co2ba.sh b.kenyon.w@gmail.com 2026-03-09
0READF:CLEAR2,F:DEFINTA-E:DEFSNGF-K:DEFSTRL-N:READF,A,J,G,N:H=F+A-1:K=0:CLS:?"Installing "N:FORI=FTOH:READB:POKEI,B:K=K+B:?".";:NEXT:?
1IFK<>GTHEN?"Bad Checksum":ELSECALLJ
2DATA59346,3614,59346,454932,"ALTERN"
3DATA205,60,245,205,49,66,42,191,245,235,33,140,4,205,143,245,124,181,202,234,231,195,222,232,205,106,245,34,189,245,42,189,245,235,33,88,2,205,119,245,205,99,245,124,181,202,19,232,42,189,245,235,33,88,2,235,66,75,8,34,187,245,195,216,231,42,189,245,235
4DATA33,0,0,205,119,245,124,181,194,114,232,205,106,245,34,185,245,42,191,245,235,33,1,0,25,34,191,245,33,239,0,235,42,189,245,235,66,75,8,101,229,33,63,0,235,42,187,245,235,66,75,8,209,93,213,33,239,0,235,42,185,245,235,66,75,8,101,229,33,63,0,235,42
5DATA187,245,235,66,75,8,209,93,213,33,1,0,205,161,245,195,216,231,42,189,245,235,33,244,1,43,205,99,245,205,143,245,124,181,202,182,232,33,0,0,34,189,245,42,191,245,235,33,1,0,25,34,191,245,33,239,0,235,42,189,245,235,66,75,8,229,33,63,0,235,42,187,245
...
51DATA0,0,0,0,0,0,0,0,0,0
```

Hex pair encoding -> 7825 bytes  
```
$ METHOD=H co2ba ALTERN.CO call >ALTERN.DO ;ls -l ALTERN.DO ;tr '\r' '\n' <ALTERN.DO
-rw-rw-r-- 1 bkw bkw 7825 Mar  9 15:29 ALTERN.DO
0'ALTERN - loader: co2ba.sh b.kenyon.w@gmail.com 2026-03-09
0READF:CLEAR2,F:DEFINTA-E:DEFSNGF-K:DEFSTRL-N:READF,A,J,G,N:E=97:M="":C=0:I=F:H=F+A-1:K=0:CLS:?"Installing "N"   0%";
1READL:FORC=1TOLEN(L)STEP2:B=(ASC(MID$(L,C,1))-E)*16+ASC(MID$(L,C+1,1))-E:POKEI,B:I=I+1:K=K+B:NEXT:?@18,USING"###%";(I-F)*100/A:IFI<=HTHEN1
2IFK<>GTHEN?"Bad Checksum":ELSECALLJ
3DATA59346,3614,59346,454932,"ALTERN"
4DATAmndmpfmndbeccklppfolcbimaemnippfhmlfmkokohmdnooimngkpfcclnpfcklnpfolcbfiacmnhhpfmngdpfhmlfmkbdoicklnpfolcbfiacolecelaiccllpfmdniohcklnpfolcbaaaamnhhpfhmlfmchcoimngkpfccljpfcklppfolcbabaabjcclppfcbopaaolcklnpfolecelaigfofcbdpaaolckllpfolecelainbfnnfcb
5DATAopaaolckljpfolecelaigfofcbdpaaolckllpfolecelainbfnnfcbabaamnkbpfmdniohcklnpfolcbpeabclmngdpfmnippfhmlfmklgoicbaaaacclnpfcklppfolcbabaabjcclppfcbopaaolcklnpfolecelaiofcbdpaaolckllpfolecelaifnobffmnemhemdniohcklppfolcbabaabjcclppfcbopaaolcklnpfbjofcbdp
...
33DATAaaaaaaaaaaaaaaaaaaaaaaaa
```

Default !yenc encoding -> 4378 bytes  
```
$ co2ba ALTERN.CO call >ALTERN.DO ;ls -l ALTERN.DO ;tr '\r' '\n' <ALTERN.DO
-rw-rw-r-- 1 bkw bkw 4378 Mar  9 15:30 ALTERN.DO
0'ALTERN - loader: co2ba.sh b.kenyon.w@gmail.com 2026-03-09
0READF:CLEAR2,F:DEFINTA-E:DEFSNGF-K:DEFSTRL-O:READF,A,J,G,N:E=128:M="!":C=0:I=F:H=F+A-1:K=0:D=0:CLS:?"Installing "N"   0%"
1READL:FORC=1TOLEN(L):O=MID$(L,C,1):IFO=MTHEND=E:NEXT:ELSEB=ASC(O)XORD:B=BXOR64:POKEI,B:D=0:I=I+1:K=K+B:NEXT:?@18,USING"###%";(I-F)*100/A:IFI<=HTHEN1
2IFK<>GTHEN?"Bad Checksum":ELSECALLJ
3DATA59346,3614,59346,454932,"ALTERN"
4DATA"�|��q!�j���a�D�ϵ<��������*�b��j���a!�B�7��#�<��S�j���a!�B�!�!�Hb�����j���a@@�7�<��2��*�b��j���aA@Yb��a�@�j���!�!�H%�a@�j���!�!�H�!��a�@�j���!�!�H%�a@�j���!�!�H�!��aA@�ᵃ��j���a�Ak�#��ϵ<����a@@b��j���aA@Yb��a�@�j���!�!�H�a@�j���!�!�H!��!��!�4��
5DATA"�j���aA@Yb��a�@�j��Y�a@�j���!�!�H!��!��!�4���a@@b���!�2/f@b��j���a[@�ϵ�#�<������!���Y@[@����q@s@w@y@}@@	@!�@!�@#@'@)@3@5@9@;@�@�@4�!���@�@!��!��!��!��_�[�!�B��������_@`@k@l@�����������������ܿڿֿοʿȿĿ<�8��@�@!��!��!��!���@�@�@�@`�^�[�!�BL���
...
19DATA"�<��':����ĵ;��˵a@@�a���<��ݵ=��ݵa���a@@�=a!�4�A��n!�b!��������\@@@@@@�������@@@@@@@@@@@@@X@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

```

XA=best
```
$ time co2ba ALTERN.CO call |wc -c
4401

real	0m0.097s
user	0m0.070s
sys	0m0.031s

$ time XA=best co2ba ALTERN.CO call |wc -c
trying all possible XA values...
XA=+122
4354

real	0m5.571s
user	0m5.541s
sys	0m0.032s
```

Override the default installed program name automatically derived from the input file name,  
and supply some extra info in a comment.  
`callba` for Tandy, or `execba` for NEC, installs the actual binary into high memory, and only a short "trigger file" in the ram filesystem.  
A short 1-line BASIC program that just does CALL or EXEC to the EXE address of the installed binary.  
TS-DOS is about 6k, and so having a .CO file of it in ram consumes a whopping 12k of ram because of the way the KC-85/Model 100 platform runs .CO files (partly forced by a limitation of the 8085 cpu).  
```
$ XA=best PN="TS-DOS" COMMENT="4.10 for TRS-80 Model 100/102" co2ba DOS100.CO callba >TS-DOS.100
$ XA=best PN="TS-DOS" COMMENT="4.10 for TANDY Model 200" co2ba DOS200.CO callba >TS-DOS.200
$ XA=best PN="TS-DOS" COMMENT="4.10 for NEC PC-8201/8300" co2ba DOSNEC.CO execba >TS-DOS.NEC
```

"Fire & Forget", `call` for Tandy or `exec` for NEC generates a loader for a binary that is only needed to be immediately executed once and not saved.  
The firmware flasher for [REX Classic](https://bitchin100.com/wiki/index.php?title=REX_Release_4.9)  
100/102 `co2ba rf149.co call >rf149.do`  
200 `co2ba rf249.co call >rf249.do`  
NEC `co2ba rfn49.co exec >rfn49.do`  

## Sending the loader to the 100
`dl -vb file.do`

<!--
or the following bash code

```
$ tsend () {
	local d=${2:-/dev/ttyUSB0} b=${3:-9600} s=([19200]=9 [9600]=8 [4800]=7 [2400]=6 [1200]=5 [600]=4 [300]=3)
	((${#1})) || { echo "${FUNCNAME[0]} FILE.DO [/dev/ttyX] [baud]" ;return ; }
	[[ -c $d ]] || { echo /dev/tty* ;return ; }
	((${#s[b]})) || { echo ${!s[*]} ;return ; }
	echo "100/200/K85/M10: RUN\"COM:${s[b]}8N1ENN"
	echo "      8201/8300: RUN\"COM:${s[b]}N81XN"
	read -p "Press [Enter] whean ready: " ;echo "Sending..."
	stty -F $d $b raw pass8 clocal cread min 1 time 0 -crtscts ixon ixoff flusho -drain
	cat $1 >$d
	printf '%b' '\x1A' >$d
}

$ tsend file.do
```
Optionally replace the cat command with:  
`tee $d <$1 |tr '\r' '\n'`  
Or better:  
`tee $d <$1 |mac2unix -q |dos2unix -q`


```
$ RLE=true co2ba ALTERN.CO call >t
RLE ENABLED
$ tsend t
100/200/K85/M10: RUN"COM:88N1ENN
      8201/8300: RUN"COM:8N81XN
Press [Enter] whean ready: 
Sending...
$
```
-->

## See also
https://github.com/hackerb9/co2do/
