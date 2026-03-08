# co2ba.sh

![co2ba.sh](co2ba.sh)  
Reads a binary .CO file and generates an ascii BASIC loader .DO file

## Usage
`co2ba FILE.CO [action] > FILE.DO`

**FILE.CO** is the input binary .CO filename that you want to bootstrap onto the portable.

**action** is what the loader should do with the .CO after it's done re-creating it on the portable:  
 call   - Immediately execute - for TANDY, Kyotronic, Olivetti  
 exec   - Immediately execute - for NEC  
 callba - Create a launcher NAME.BA - for TANDY, Kyotronic, Olivetti  
 execba - Create a launcher NAME.BA - for NEC  
 savem  - Save NAME.CO - for TANDY, Kyotronic, Olivetti  
 bsave  - Save NAME.CO - for NEC  
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
BASAFE=false   # add 0xFF to unsafe list so that the BASIC can be saved as .BA
METHOD=A       # which encoding scheme: A=ModifiedAdolph B=AwithoutIF H=hexpairs I=ints
```
Methods A & B generate identical data, just different loader implementations.

## Examples
<!--
`co2ba TSLOAD.CO savem "TSLOAD for TANDY 200 - Travelling Software" >TSLOAD.200`
-->

[RAM100.DO](https://github.com/bkw777/NODE_DATAPAC/tree/main/software/RAMDSK/RAM100)
`co2ba RAM100.CO savem >RAM100.DO`

[ALTERN.DO](https://github.com/LivingM100SIG/Living_M100SIG/blob/main/M100SIG/Lib-07-UTILITIES/ALTERN.100)
`FIRST=50 LINE_GAP=5 LINE_LEN=74 BASAFE=true co2ba ALTERN.CO call >ALTERN.DO`

## See also
https://github.com/hackerb9/co2do/
