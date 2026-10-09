0'TEENY - TRS-80 Model 100/102 North America - github.com/bkw777/dl2 - loader: co2ba.sh b.kenyon.w@gmail.com 2026-10-09
0READF:CLEAR12,F:DEFINTA-E,Q,G,K:DEFSNGF,H-J:DEFSTRL-O:READF,A,J,G,N,E,Q:M="!":C=0:I=F:H=F+A-1:K=0:D=0:CLS:?USING"Installing \    \   0%";N
1READL:FORC=1TOLEN(L):O=MID$(L,C,1):IFO=MTHEND=E:NEXT:ELSEB=ASC(O)XORD:B=BXORQ:D=0:POKEI,B:I=I+1:K=KXORB:NEXT:?@18,USING"###%";(I-F)*100/A:IFI<=HTHEN1
2IFK<>GTHEN?"Bad Checksum":ELSEM=CHR$(34):L="X.DO":OPENLFOROUTPUTAS1:?#1,"0CLEAR0,"F":CALL"J":MENU":CLOSE1:?"Please type:":?"KILL"M""L:?"SAVE"M""N:LOADL
3DATA62240,720,62240,15,"TEENY",128,64
4DATA"a@@ybF½a!š!šbJ½jF½¹ïrH½am³¥aþµâQrê¿!„!†a4³b!’¶aÆ¶vbO!Œ!†azÅ¶¾!‘Š×!—¾!“Š	µµ^´Š7³±–!‹ŠY´}Š9´^FA^BA^DA^HA^JA^LA^NA^PzH½÷Šß³•jB½!„!j@½ß+‘ažµV@Y>§—§a™µâQƒm³[:ó‚ò³‰yx!Žq!„aL½7cv@‰aL½>cÆ!—>!Ÿ!‡c÷:Šœ³Æc]‚–³o7~EÀ!‡aJ½>r.š=³cE‚¥³aL½!œ6P´P´!LP´M‚
5DATA"¿³ïpaM½!Žc>÷‰>-‚=³7c‰~EƒÍµ‹.Vnò³aù³w¦W~GÍµïþ³vZcQÇ¶FI)tFOv`cE‚!ƒ´v!†c0†³!‡zL½¾RŠÔµ8÷‰^A~Aþ³7c3ƒÐµ‘~Dþ³1c!)tƒÐµï`‚7³aÃ³b!’¶¤`¥!€µjg½!…!Œ) bD½K!Ÿ|C}‚Ù´!—aMµYYYYY>µQvµ•c!žc!–•c!žc&+!žc!– )bB½«b@½¥-+šÃ³aH½tµ´jD½«¡zL½!F@:ó9rL½ˆ÷¼º´[¥aN½
6DATA"I>¡7cLƒœ´^C!Ÿ´¥•~Cþ³!Ž´ï!‡!rL½‘¡‰àoµð»€\µî»À_µÚ¹vZ‰ï7c7jB½«jî»Ybî»‰j@½bð»‰±j@½«[¡ƒybzÙ¼–!ƒ°~B‰^´‚7³zÙ¼¾!‚š4³¾!…’4³ï`Š7³!€µ!~BÑ…•«Ýa‘=Ó/<Ú'LckM‚9µ=ôˆ•¥!´‘¡Ýµ~Bþ³†³ˆ¦°___!ŸƒË³¥¥aÀ@Ÿ’ùµ!!„¡I£«AÀ¿I¥+´‘¡ƒÝµ!‹¡ƒ*´~`!ƒ`!†!†!†!†!†!†n!˜!˜`h
7DATA"!ƒ}!‹!Œ!“!‘iMJ~`@`!…22MJ@!†!†!•!…!“!Ž	!!—!!„!†!Ž!„!ˆ!—
