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
COMMENT=""     # extra text inserted into the first line comment
FIRST=0        # first line number
LINE_GAP=1     # line number increment
LINE_LEN=256   # length of DATA lines
METHOD=Y       # which encoding scheme: Y=!yenc B=YwithoutIF H=hexpairs I=ints
UNSAFE="0 1 2 3 4 5 6 7 8 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 34"
               # (Y) list of byte values that need to be encoded
EDITSAFE=true  # (Y) add 127 to the unsafe list so the output can be opened in EDIT
RLE=false      # (Y) enable run-length encoding (experimental)
EP='!'         # (Y) escape prefix - character that indicates the next byte is encoded
RP=' '         # (Y) rle prefix - character that indicates the next byte is how many copies of the previous byte to append here
XA=^64         # (Y) initial transform applied to all bytes - best,0,^###,+###
XB=^128        # (Y) encoding transform applied to unsafe bytes - ^###,+###
YENC=false     # output standard yEnc, shorthand for ESC='=' XA='+42' XB='+64'
```
<!-- does not work
   "?OD error in 1"
CARAT=false    # output standard carat encoding, shorthand for ESC='^' XA=0 XB="+64"
-->

## Encoding schemes
Method Y "!yenc" (default):  
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

Method B:  
  Identical data to Y.  
  The difference is the inner loop in BASIC to decode bytes does not use any IF branching.  
  It's actually slower so don't use it.  
  It's just here because if it wasn't, you or I would try to do it again.  
  Also it was hard to figure out so I want to keep it as a reference for tricks. And who knows maybe it can get better.

Method H:  
  Hex pairs a-la James Yi / Kurt McCullum / others.  
  Hex pairs, but using a single contiguous range of ascii values like a-p for the alphabet instead of 0-9A-F.  
  0x00 = aa, 0x01 = ab, ... 0xFF = pp  
  A lot of old loaders use this because the code is small and simple, and the output is at least better than plain ints.

Method I:  
  Plain comma seperated integers.  
  The simplest possible way to put binary into DATA statements and read them.  
  It's useful for very small payloads because the BASIC to load it is almost nothing.

All methods include a rolling + incrementing xor checksum to catch corrupt serial transfers.

For !yenc, you can get a slightly smaller output file by using `XA=best` .  
This will internally try all possible XA values with both xor & rot (^0-^255 +0-+255) and pick the best.  
This will take several seconds and generally produce a smaller file, but not greatly, which is why it's not the default.  
The best value will be different for every input file.

## RLE
*Experimental* Only supported in METHOD=Y  
`$RLE=true XA=0 co2ba call ALTERN.CO >ALTERN.DO`

Currently only works if XA=0

The BASIC code is larger and slower, and the file size is usually larger because of XA=0.  
Also because enabling RLE means adding another value (the RP character) to the unsafe list,  
so all occurrances of that value in the data now need to be encoded.  

So you usually don't want this.

The encoding scheme is:  

  RP N
  
Wher RP is the rle-prefix byte ' ' (0x20, a single space)  
and N is a a single byte (or encoded !+byte) value for the number of times to copy the byte that came before the RP.

So if the source input has a run of 10 nulls, that becomes:  
0x21  '!'/EP esc-prefix because the first byte of the run is output normally, and nuls are encoded.  
0x80  nul after transforming with xor128, these 2 bytes make up the single nul.  
0x20  ' '/RP rle-prefix, says to remember the previous byte (nul) and read the next byte as a number.  
0x21  '!' EP because the number will be 9, and a 9 byte needs to be encoded.
0x89  9 after transforming by xor128.

On reading the RP and then the 9, write 9 copies of nul, leaving a run of 10 at the end.  
Number is a single byte, so runs longer than 255 bytes use multiple rle sequences.  

Short runs are inefficient currently because of the XA bug. If XA didn't have to be disabled  
when using RLE, then the XA would shift all the low byte values up to where they don't need to be encoded.  
The same goes for the byte value being copied too. More often the value happens to be null,  
so the XA shifts that too. Resulting that an rle-sequence of up to 128 nulls would only need 3 total bytes, the first null, the RP, and a byte for the number 127.

But only because you currently have to disable XA with RLE, if the byte being copied is a low number like null, then it needs to be encoded.  
And if the run is under 35 bytes long, then the byte for the number also needs to be encoded,  
so the entire sequence takes 5 bytes.

<!-- 

 
While encoding:
  - if the current byte (before encoding with EP) matches the previous byte,  
  increment the rl counter instead of writing more copies  
  - when the current byte no longer matches the previous byte, or rl reaches 255, write the rle sequence.  
  (if rl<2, or even <3, just write multiple copies of the byte instead of the rle sequence) 

While decoding:
  - Each time you actually poke a byte (IE after doing all necessary decoding to arrive at the actual data),  
  Save the current byte to a previous-byte variable (overwrite).  
  - If you encounter RP,  
    - read the next byte, decode as normal, and interpret the decoded value as a number.  
    - write number more copies of previous-byte.

number is a single byte, and so can only range from 0-255.  
number is one less than the total string of dupes.  
runs longer than 255 simply use multiple rle sequences.  

Both source-byte and number are normal data that are encoded as necessary, so it may actually be:  

  EP shifted-source-byte RP EP shifted-number


Normally and ideally, the default XA would shift all data values up before the main processing,  
such that nulls become 64s which don't need to be encoded with EP.  
And that would also mean that short rle runs with a small number would also not need to be encoded.  
So short rle runs would only need 2 bytes.  

But only because currently the BASIC decoder isn't fully working yet, XA must be disabled,  
and that means short runs have a number that is small, which as a byte value needs to be encoded,  
so short runs actually wind up needing more characters to encode than long runs.

So untill the XA problem is fixed, RLE is very inefficiean and doesn't actually gain much.

In fact it often makes the file *larger*, because the BASIC code to decode is larger,  
and disabling XA hurts a lot. The BASIC code also runs slower.
-->

## Examples
<!--
`co2ba TSLOAD.CO savem "TSLOAD for TANDY 200 - Travelling Software" >TSLOAD.200`
-->

[RAM100.DO](https://github.com/bkw777/NODE_DATAPAC/tree/main/software/RAMDSK/RAM100)  
`co2ba RAM100.CO savem >RAM100.DO`

[ALTERN.DO](https://github.com/LivingM100SIG/Living_M100SIG/blob/main/M100SIG/Lib-07-UTILITIES/ALTERN.100)  
`FIRST=50 LINE_GAP=5 LINE_LEN=74 EDITSAFE=true co2ba ALTERN.CO call >ALTERN.DO`

## Results

Sample input .CO file = 3620 bytes  
```
$ ls -l ALTERN.CO
-rw-rw-r-- 1 bkw bkw 3620 Feb 28 14:10 ALTERN.CO
```

(All of these actually consume 60 bytes less than the file size, because the first line 0 gets overwritten on the receiving machine.)

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

## See also
https://github.com/hackerb9/co2do/
