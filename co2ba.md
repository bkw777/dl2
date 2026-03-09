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

## Examples
<!--
`co2ba TSLOAD.CO savem "TSLOAD for TANDY 200 - Travelling Software" >TSLOAD.200`
-->

[RAM100.DO](https://github.com/bkw777/NODE_DATAPAC/tree/main/software/RAMDSK/RAM100)  
`co2ba RAM100.CO savem >RAM100.DO`

[ALTERN.DO](https://github.com/LivingM100SIG/Living_M100SIG/blob/main/M100SIG/Lib-07-UTILITIES/ALTERN.100)  
`FIRST=50 LINE_GAP=5 LINE_LEN=74 EDITSAFE=true co2ba ALTERN.CO call >ALTERN.DO`

## See also
https://github.com/hackerb9/co2do/
