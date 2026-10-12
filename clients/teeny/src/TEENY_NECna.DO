0'TEENY - NEC PC-8201A/PC-8300 North America - github.com/bkw777/dl2 - loader: co2ba.sh b.kenyon.w@gmail.com 2026-10-12
0READF:CLEAR12,F:DEFINTA-E,Q,G,K:DEFSNGF,H-J:DEFSTRL-O:READF,A,J,G,N,E,Q:M="!":C=0:I=F:H=F+A-1:K=0:D=0:CLS:?"Installing",N:LOCATE22,0:?"  0%"
1READL:FORC=1TOLEN(L):O=MID$(L,C,1):IFO=MTHEND=E:NEXT:ELSEB=ASC(O)XORD:B=BXORQ:D=0:POKEI,B:I=I+1:K=KXORB:NEXT:LOCATE22,0:?USING"###%";(I-F)*100/A:IFI<=HTHEN1
2IFK<>GTHEN?"Bad Checksum":ELSEM=CHR$(34):L="X.DO":OPENLFOROUTPUTAS1:?#1,"0CLEAR0,"F":EXEC"J":MENU":CLOSE1:?"Please type:":?"KILL"M""L:?"SAVE"M""N:LOADL
3DATA61704,632,61704,94,"TEENY",128,64
4DATA"x@´bU±a!ö!öbàªq@@ÔrÜªaT±•a ≥ç!¢Tr(æçÍ!áa!ö±bæ≥a#¥vbç˝!ççzcz!¢¥æ!ëä!Ö!ñæ!ìä•≤µç√±ä!ö±±ñ!ãäI≤}ä[≤zÜª˜ä,±jÇª!Ñ!çjÄªç|,a;≥ç!¢TÉT±y!éxq!é!éaäª7cv@âçË/Vnç!Ä0a5±wç!é\~Gçi≥Ôç;±vZcQ$¥FIç∑qFOv`cEÇË±v!Üc0aäª>c∆!ó>!ü!ác˜:äâ±∆c]ÇÉ±o7~E¿!áaàª>ç˛.ö!ö±cEÇí±aäªç•6çΩ
5DATA"±çΩ±!èLçΩ±MÇ¨±Ôpaãª!éc>˜âçì-ö!ö±Ç!ö±7câ~EÉi≥^A~Aç;±7c3Él≥ç€bÇ!ö±çìb•çú≤j•ª!Ö!å) bÑªK!ü|C}Çu≤!óaÈ≤YYYYY>µQí≤ïc!ûc!ñïc!ûc&+!ûc!ñ )bÇª´bÄª•çJ,ö!ö±aÜªtç—≤jÑª´°zäª!èF@:Û9räªà˜º÷≤[•aåªI>°7cLÉ8≤^CçP≤•ï~Cç;±çÛ±Ô!á!èräªë°â‡ã≤£∫Ä¯≤°∫¿˚≤1∏vZâÔ7c7jÇª´j°∫Yb°∫âjÄ
6DATA"ªb£∫â±jÄª´[°Éudz>ªñ!É∞~Bâç√±Ç!ö±z>ªæ!Çö!ö±æ!Öí!ö±ç€bä!ö±çú≤!è~B—Öï´çŸcë=”/<⁄'ÅLckMÇU≥=Ùàï•çN≤ë°çs≥~Bç;±çÛ±àÉ!ö±••a¿@üí!è≥!ç!Ñ°I£´A¿øI•ç!í≥ë°És≥!ã°ë~Dç;±1c!Åç∑qÉl≥~`!É`!Ü!Ü!Ü!Ü!Ü!Ün!ò!ò`h!É}!ã!å!ì!ëiMJ~`@!Ö!íMJ@
