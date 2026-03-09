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
Available options and their default values:  
```
FIRST=0        # first line number
LINE_GAP=1     # line number increment
LINE_LEN=256   # length of DATA lines
UNSAFE="0 1 2 3 4 5 6 7 8 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 34"
               # list of byte values that need to be encoded
EDITSAFE=false # add 127 to unsafe list so FILE.DO can be opened in EDIT
METHOD=A       # which encoding scheme: A=!code B=AwithoutIF H=hexpairs I=ints
ESC='!'        # character that indicates the next byte is encoded
XOR=128        # encode unsafe bytes by applying xor this value
ROT=0          # transform all bytes before encoding: add this value, >255 wraps around to 0+
XROT=64        # transform all bytes before encoding: xor this value
```

METHOD A:
  Modified Stephen Adolph
  Most bytes simply copy unchanged from input to output.
  Only for unsafe bytes apply a simple transform (xor128, aka flip the high bit) and prefix with '!'  

METHOD B:
  Identical data to A.
  The loader BASIC code is just written a different way to try to make it faster.
  It's actually slower so don't use it.
  It's just here to head off trying to write the same "improvement" if you didn't know it was already done.

METHOD H:
  Quasi-hex pairs a-la James Yi / Kurt McCullum.
  Hex pairs but using a more convenient alphabet.

Method I:
  The simplest possible way to put binary into DATA statements and read them.
  Plain comma seperated ints.
  It's useful for very small payloads because the BASIC to load it is almost nothing.

The purpose of ROT or XROT is like yEnc which adds 42 to everything to shift NULs
to where they don't need to be encoded, because strings of nuls are common.
It ends up reducing the encoded file size (on average)
simply because fewer bytes need to be encoded (on average).
This only applies to method A (or B), no effect on H or I.

## Examples
<!--
`co2ba TSLOAD.CO savem "TSLOAD for TANDY 200 - Travelling Software" >TSLOAD.200`
-->

[RAM100.DO](https://github.com/bkw777/NODE_DATAPAC/tree/main/software/RAMDSK/RAM100)  
`co2ba RAM100.CO savem >RAM100.DO`

[ALTERN.DO](https://github.com/LivingM100SIG/Living_M100SIG/blob/main/M100SIG/Lib-07-UTILITIES/ALTERN.100)  
`FIRST=50 LINE_GAP=5 LINE_LEN=74 EDITSAFE=true co2ba ALTERN.CO call >ALTERN.DO`

## Results

```
$ ls -l ALTERN.CO
-rw-rw-r-- 1 bkw bkw 3620 Feb 28 14:10 ALTERN.CO
```

Simple INT encoding  
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

Classic hex pair encoding used by many old loaders  
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

Default new encoding without the extra yEnc-like rotation before encoding  
```
$ XROT=0 co2ba ALTERN.CO call >ALTERN.DO ;ls -l ALTERN.DO ;tr '\r' '\n' <ALTERN.DO
-rw-rw-r-- 1 bkw bkw 5305 Mar  9 15:30 ALTERN.DO
0'ALTERN - loader: co2ba.sh b.kenyon.w@gmail.com 2026-03-09
0READF:CLEAR2,F:DEFINTA-E:DEFSNGF-K:DEFSTRL-O:READF,A,J,G,N:E=128:M="!":C=0:I=F:H=F+A-1:K=0:D=0:CLS:?"Installing "N"   0%"
1READL:FORC=1TOLEN(L):O=MID$(L,C,1):IFO=MTHEND=E:NEXT:ELSEB=ASC(O)XORD:POKEI,B:D=0:I=I+1:K=K+B:NEXT:?@18,USING"###%";(I-F)*100/A:IFI<=HTHEN1
2IFK<>GTHEN?"Bad Checksum":ELSECALLJ
3DATA59346,3614,59346,454932,"ALTERN"
4DATA"�<��1B*���!��!�͏�|��������j�!���*���!�X!��w��c�|��!��*���!�X!��BK!�!������*���!�!�!��w�|��r��j�!���*���!�!�!�!�!���!��!��*���BK!�e�!�?!��*���BK!��]�!��!��*���BK!�e�!�?!��*���BK!��]�!�!�!�͡����*���!��!�+�c�͏�|�ʶ�!�!�!�!���*���!�!�!�!�!���!��!��*�
5DATA"��BK!��!�?!��*���BK!�]�U�Lt���*���!�!�!�!�!���!��!��*��!��!�?!��*���BK!�]�U�Lt���!�!�!�!����Bro&!�!���*���!�!�!�͏��c�|����͗W��!�!�!�!�����1!�3!�7!�9!�=!�?!�I!�K!�a!�c!�g!�i!�s!�u!�y!�{!��!��!�t�[��!��!�Q�L�H�D�!��!��Y!���������!�!� !�+!�,!��������
...
23DATA"!�!�!�!�!�!�!�!�!�!�!�!�!�!�!�!�!�!�!�
```

Default new encoding  
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
