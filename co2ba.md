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
You don't need to change any of these. They exist and are documented here just for flexability and completeness, but you never need to change any of these just to use the app. Well *maybe* EDITSAFE=true.  
Available settings and their default values:  
```
FIRST=0        # first line number
LINE_GAP=1     # line number increment
LINE_LEN=256   # length of DATA lines
UNSAFE="0 1 2 3 4 5 6 7 8 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 34"
               # list of byte values that need to be encoded
EDITSAFE=false # add 127 to unsafe list so FILE.DO can be opened in EDIT
METHOD=A       # which encoding scheme: A=!yenc B=AwithoutIF H=hexpairs I=ints
ESC='!'        # character that indicates the next byte is encoded
XOR=128        # encoding transform unsafe bytes B using B^XOR
ROT=0          # initial transform all bytes B using (B+ROT)%256
XROT=64        # initial transform all bytes B using B^XROT
```

## Encoding schemes
Method A "!yenc" (default):  
The main idea of this one comes from Stephen Adolph, modified by HackerB9 & Brian White.  
It is very similar to [yEnc](http://www.yenc.org/yenc-draft.1.3.txt).  
  - Apply a simple transform the same way to all input bytes. yenc does (b+42)%256, we do b^64.  
  - For each (transformed) byte:  
    - If a byte is safe, copy it to output without any changes.  
    - If a byte is unsafe,  
    Output an escape character prefix. yenc uses '=', we use '!'  
    Apply another simple transform to make a safe byte. yenc does (b+64)%256, we do b^128.

Method B:  
  Identical data to A.  
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

Simple INT encoding -> 12,478 bytes  
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

Default new encoding -> 4378 bytes  
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

## See also
https://github.com/hackerb9/co2do/
