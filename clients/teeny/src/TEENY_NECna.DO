0'TEENY - NEC PC-8201A/PC-8300 North America - github.com/bkw777/dl2 - loader: co2ba.sh b.kenyon.w@gmail.com 2026-10-22
0READF:CLEAR12,F:DEFINTA-E,Q,G,K:DEFSNGF,H-J:DEFSTRL-O:READF,A,J,G,N,E,Q:M="!":C=0:I=F:H=F+A-1:K=0:D=0:CLS:?"Installing "N:LOCATE18,0:?"  0%"
1READL:FORC=1TOLEN(L):O=MID$(L,C,1):IFO=MTHEND=E:NEXT:ELSEB=ASC(O)XORD:B=BXORQ:D=0:POKEI,B:I=I+1:K=KXORB:NEXT:LOCATE18,0:?USING"###%";(I-F)*100/A:IFI<=HTHEN1
2IFK<>GTHEN?"Bad Checksum":ELSEM=CHR$(34):L="X.DO":OPENLFOROUTPUTAS1:?#1,"0CLEAR0,"F":EXEC"J":MENU":CLOSE1:?"Please type:":?"KILL"M""L:?"SAVE"M""N:LOADL
3DATA61718,618,61718,249,"TEENY",128,64
4DATA"x@´bc±a!ö!öbàªq@@ÔrÜªab±•a ≥ç!¢Tr(æçÍ!áa(±bæ≥a#¥vbç˝!ççzcz!¢¥æ!ëä!Ö!ñæ!ìäË≤µç√±ä(±±ñ!ãäΩ±}äB≤zÜª˜ä:±jÇª!Ñ!çjÄªç|,a;≥ç!¢TÉb±çË/Vnç!Ä0a!ö≥wç!é\~Gç¨≤Ôçn≥vZcQ$¥FIç∑qFOv`cEÇË±v!Üc0aäª>c∆!ó>!ü!ác˜:äâ±∆c]ÇÉ±o7~E¿!áaàª>ç˛.ö(±cEÇí±aäªç•6ç!ˇ≥ç!ˇ≥!èLç!ˇ≥MÇ¨±Ôp
5DATA"aãª!éc>˜â~EÉ¨≤ç€bÇ(±çìb•çv≥j•ª!Ö!å) bÑªK!ü|C}Ç\≤!óa!ã≥YYYYY>µQﬁ≤ïc!ûc!ñïc!ûc&+!ûc!ñ )bÇª´bÄª•çJ,ö(±aÜªtç/≤jÑª´°:Ûà9˜º4≤[•aåªI>°7cLÉ!ô≤^Cç∏≤•ï~Cçn≥çÛ±Ô!á!èë°âvZâÔ7c7jÇª´j°∫Yb°∫âjÄªb£∫â±jÄª´[°Éudç√±Ç(±z>ªæ!Çö(±æ!Öí(±ç€bä(±çv≥!è~B—Öï´çŸcë=”/<⁄'ÅLckMÇò≤
6DATA"=Ùàï•ç∂≤ë°çC≥~Bçn≥çÛ±àÉ(±^A~Açn≥7c3ÉØ≤••a¿@üí]≥A¿@°I£´H•ç`≥ë°ÉC≥!ãëë~Dçn≥1c!Åç∑qÉØ≤aäª7cv@âz>ªñ!É∞~Bâçì-ö(±Ç(±7câ‡◊≤£∫Äƒ≤°∫¿«≤1∏y!éxq!é!é~`!É`!Ü!Ü!Ü!Ü!Ü!Ün!ò!ò`h!É}!ã!å!ì!ëiMJ~`@!Ö!íMJ@
