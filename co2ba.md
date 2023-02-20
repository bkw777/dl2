# co2ba.sh

Reads a binary .CO file and generates an ascii BASIC loader .DO file

The general usage is  
```co2ba FILE.CO [action] [comment] > FILE.DO```

**FILE.CO** is the input binary .CO filename that you want to bootstrap onto the portable.

**action** is what the loader should do with the .CO after it's done re-creating it on the portable:  
 call  - Immediately execute - for TANDY, Kyotronic, Olivetti  
 exec  - Immediately execute - for NEC  
 savem - Save FILE.CO - for TANDY, Kyotronic, Olivetti  
 bsave - Save FILE.CO - for NEC  
 Otherwise if the option is not given, or any other value than these, the loader will only print a message showing the Top, End, and Exec addresses of the loaded binary.  

**comment** is an optional custom replacement text for the first half of the line #0 comment.  
 By default a basic comment giving the name of the .CO file is generated.  
 The 2nd half of the line always has co2ba.sh itself and the date it was run to generate the loader.  
 You can use this to give more info about the payload than just the filename.  

**FILE.DO** is the output ascii BASIC .DO filename.

For example, to generate TSLOAD.200  
```co2ba TSLOAD.CO savem "TSLOAD for TANDY 200 - Travelling Software" >TSLOAD.200```
