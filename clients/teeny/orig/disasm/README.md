Asm disassembly & reconsruction of buildable asm source for TEENY for TANDY 200 and NEC PC-8201A

Labelled but not annotated. See the main 100/M10/K85 source for annotation.

```
$ make clean all
rm -f *.o *.bin *.sym *.lis *.map *.def *.CO *.DO
z88dk-z80asm -m8085_strict -no-synth  -DZ88DK=24602  -b -o=TNY200.CO TNY200.S85 && \
z88dk-z80asm -m8085_strict -no-synth  -DZ88DK=24602  -DPRGLEN=$(wc -c <TNY200.CO) -b -o=TNY200.CO TNY200.S85
case "TNY200" in *NEC*) o=bsave ;; *) o=savem ;; esac ;co2ba TNY200.CO $o >TNY200.DO
z88dk-z80asm -m8085_strict -no-synth  -DZ88DK=24602  -b -o=TNYNEC.CO TNYNEC.S85 && \
z88dk-z80asm -m8085_strict -no-synth  -DZ88DK=24602  -DPRGLEN=$(wc -c <TNYNEC.CO) -b -o=TNYNEC.CO TNYNEC.S85
case "TNYNEC" in *NEC*) o=bsave ;; *) o=savem ;; esac ;co2ba TNYNEC.CO $o >TNYNEC.DO
$ make verify
Built TNY200.CO is identical to reference ../TNY200.CO.
Built TNYNEC.CO is identical to reference ../TNYNEC.CO.
$ 
```
