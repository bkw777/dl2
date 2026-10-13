0'TEENY - NEC PC-8201A/PC-8300 North America - github.com/bkw777/dl2 - loader: co2ba.sh b.kenyon.w@gmail.com 2026-10-13
0READF:CLEAR12,F:DEFINTA-E,Q,G,K:DEFSNGF,H-J:DEFSTRL-O:READF,A,J,G,N,E,Q:M="!":C=0:I=F:H=F+A-1:K=0:D=0:CLS:?"Installing",N:LOCATE22,0:?"  0%"
1READL:FORC=1TOLEN(L):O=MID$(L,C,1):IFO=MTHEND=E:NEXT:ELSEB=ASC(O)XORD:B=BXORQ:D=0:POKEI,B:I=I+1:K=KXORB:NEXT:LOCATE22,0:?USING"###%";(I-F)*100/A:IFI<=HTHEN1
2IFK<>GTHEN?"Bad Checksum":ELSEM=CHR$(34):L="X.DO":OPENLFOROUTPUTAS1:?#1,"0CLEAR0,"F":EXEC"J":MENU":CLOSE1:?"Please type:":?"KILL"M""L:?"SAVE"M""N:LOADL
3DATA61716,620,61716,56,"TEENY",128,64
4DATA"x@´ba±a!ö!öbàªq@@ÔrÜªa`±•a ≥ç!¢Tr(æçÍ!áa&±bæ≥a#¥vbç˝!ççzcz!¢¥æ!ëä!Ö!ñæ!ìäÊ≤µç¡±ä&±±ñ!ãäª±}ä@≤zÜª˜ä8±jÇª!Ñ!çjÄªç|,a;≥ç!¢TÉ`±çË/Vnç!Ä0a!ö≥wç!é\~Gç™≤Ôçn≥vZcQ$¥FIç∑qFOv`cEÇÊ±v!Üc0aäª>c∆!ó>!ü!ác˜:äá±∆c]ÇÅ±o7~E¿!áaàª>ç˛.ö&±cEÇê±aäªç•6ç!ˇ≥ç!ˇ≥!èLç!ˇ≥MÇ™±Ôp
5DATA"aãª!éc>˜â~EÉ™≤ç€bÇ&±çìb•çv≥j•ª!Ö!å) bÑªK!ü|C}ÇZ≤!óa!ã≥YYYYY>µQ‹≤ïc!ûc!ñïc!ûc&+!ûc!ñ )bÇª´bÄª•çJ,ö&±aÜªtç-≤jÑª´°:Ûà9˜º2≤[•aåªI>°7cLÉ!ó≤^Cç∂≤•ï~Cçn≥çÒ±Ô!á!èë°âvZâÔ7c7jÇª´j°∫Yb°∫âjÄªb£∫â±jÄª´[°Éudç¡±Ç&±z>ªæ!Çö&±æ!Öí&±ç€bä&±çv≥!è~B—Öï´çŸcë=”/<⁄'ÅLckMÇñ≤
6DATA"=Ùàï•ç¥≤ë°çA≥~Bçn≥çÒ±àÉ&±^A~Açn≥7c3É≠≤••a¿@üí]≥!ç!Ñ°I£´A¿øI•ç`≥ë°ÉA≥!ã°ë~Dçn≥1c!Åç∑qÉ≠≤aäª7cv@âz>ªñ!É∞~Bâçì-ö&±Ç&±7câ‡’≤£∫Ä¬≤°∫¿≈≤1∏y!éxq!é!é~`!É`!Ü!Ü!Ü!Ü!Ü!Ün!ò!ò`h!É}!ã!å!ì!ëiMJ~`@!Ö!íMJ@
