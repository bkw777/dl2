0'TEENY - TRS-80 Model 100/102 North America - github.com/bkw777/dl2 - loader: co2ba.sh b.kenyon.w@gmail.com 2026-10-10
0READF:CLEAR12,F:DEFINTA-E,Q,G,K:DEFSNGF,H-J:DEFSTRL-O:READF,A,J,G,N,E,Q:M="!":C=0:I=F:H=F+A-1:K=0:D=0:CLS:?USING"Installing \    \   0%";N
1READL:FORC=1TOLEN(L):O=MID$(L,C,1):IFO=MTHEND=E:NEXT:ELSEB=ASC(O)XORD:B=BXORQ:D=0:POKEI,B:I=I+1:K=KXORB:NEXT:?@18,USING"###%";(I-F)*100/A:IFI<=HTHEN1
2IFK<>GTHEN?"Bad Checksum":ELSEM=CHR$(34):L="X.DO":OPENLFOROUTPUTAS1:?#1,"0CLEAR0,"F":CALL"J":MENU":CLOSE1:?"Please type:":?"KILL"M""L:?"SAVE"M""N:LOADL
3DATA62319,641,62319,188,"TEENY",128,64
4DATA"a@@ybFΩa!ö!öbJΩjFΩπÔrHΩa<≥•aèµç‚QrÍøç!Ñ!ÜaÉ≥b!í∂a∆∂vbçO!åç!Üaz≈∂æ!ëä◊!óæ!ìä!óµµç≤≥äÉ≥±ñ!ãä5¥}ä«¥zHΩ˜äï≥jBΩ!Ñ!çj@Ωçﬂ+a™µç‚QÉ<≥[:ÛÇû≥âyx!éq!ÑaLΩ7cv@âçã.Vnçû≥a•≥wç¶W~Gç€µÔç™≥vZcQ«∂FIç)tFOv`cEÇW¥v!Üc0aLΩ>c∆!ó>!ü!ác˜:äx¥∆c]Çr¥o7~E¿!áaJΩ>çr.öÉ≥cEÇ!Å¥aLΩç
5DATA"!ú6ç,¥ç,¥!èLç,¥MÇ!õ¥ÔpaMΩ!éc>˜âç>-ÇÉ≥7câ~EÉ€µ^A~Aç™≥7c3ÉﬁµçÔ`ÇÉ≥aÉ≥b!í∂ç§`•ç!éµjgΩ!Ö!å) bDΩK!ü|C}ÇÁ¥!óa[µYYYYY>µQ!Ñµïc!ûc!ñïc!ûc&+!ûc!ñ )bBΩ´b@Ω•ç-+öÉ≥aHΩtçCµjDΩ´°zLΩ!èF@:Û9rLΩà˜ºHµ[•aNΩI>°7cLÉ™¥^Cç<¥•ï~Cç™≥çb¥Ô!á!èrLΩë°â‡}µªÄjµÓª¿mµ⁄πvZâÔ7c7jBΩ´jÓ
6DATA"ªYbÓªâj@Ωbªâ±j@Ω´[°ÉybzŸºñ!É∞~Bâç≤≥ÇÉ≥zŸºæ!ÇöÉ≥æ!ÖíÉ≥çÔ`äÉ≥ç!éµ!è~B—Öï´ç›aë=”/<⁄'ÅLckMÇ«µ=Ùàï•ç:¥ë°ç‚µ~Bç™≥çb¥à••a¿@üí˛µ!ç!Ñ°I£´A¿øI•çÅµë°É‚µ!ã°ë~Dç™≥1c!Åç)tÉﬁµ~`!É`!Ü!Ü!Ü!Ü!Ü!Ün!ò!ò`h!É}!ã!å!ì!ëiMJ~`@!Ö!í!íMJ@
