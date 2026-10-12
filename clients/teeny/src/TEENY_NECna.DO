0'TEENY - NEC PC-8201A/PC-8300 North America - github.com/bkw777/dl2 - loader: co2ba.sh b.kenyon.w@gmail.com 2026-10-12
0READF:CLEAR12,F:DEFINTA-E,Q,G,K:DEFSNGF,H-J:DEFSTRL-O:READF,A,J,G,N,E,Q:M="!":C=0:I=F:H=F+A-1:K=0:D=0:CLS:?"Installing",N:LOCATE22,0:?"  0%"
1READL:FORC=1TOLEN(L):O=MID$(L,C,1):IFO=MTHEND=E:NEXT:ELSEB=ASC(O)XORD:B=BXORQ:D=0:POKEI,B:I=I+1:K=KXORB:NEXT:LOCATE22,0:?USING"###%";(I-F)*100/A:IFI<=HTHEN1
2IFK<>GTHEN?"Bad Checksum":ELSEM=CHR$(34):L="X.DO":OPENLFOROUTPUTAS1:?#1,"0CLEAR0,"F":EXEC"J":MENU":CLOSE1:?"Please type:":?"KILL"M""L:?"SAVE"M""N:LOADL
3DATA61704,632,61704,145,"TEENY",128,64
4DATA"x@´bU±a!ö!öbJºq@@ÔrHºaT±•a ≥ç!¢Tr(æçÍ!áa!ö±bæ≥a#¥vbç˝!ççzcz!¢¥æ!ëä!Ö!ñæ!ìä•≤µç√±ä!ö±±ñ!ãäI≤}ä[≤zHº˜ä,±jBº!Ñ!çj@ºç|,a;≥ç!¢TÉT±y!éxq!é!éaLº7cv@âçË/Vnç!Ä0a5±wç!é\~Gçi≥Ôç;±vZcQ$¥FIç∑qFOv`cEÇË±v!Üc0aLº>c∆!ó>!ü!ác˜:äâ±∆c]ÇÉ±o7~E¿!áaJº>ç˛.ö!ö±cEÇí±aLºç•6çΩ
5DATA"±çΩ±!èLçΩ±MÇ¨±ÔpaMº!éc>˜âçì-ö!ö±Ç!ö±7câ~EÉi≥^A~Aç;±7c3Él≥ç€bÇ!ö±çìb•çú≤jgº!Ö!å) bDºK!ü|C}Çu≤!óaÈ≤YYYYY>µQí≤ïc!ûc!ñïc!ûc&+!ûc!ñ )bBº´b@º•çJ,ö!ö±aHºtç—≤jDº´°zLº!èF@:Û9rLºà˜º÷≤[•aNºI>°7cLÉ8≤^CçP≤•ï~Cç;±çÛ±Ô!á!èrLºë°â‡ã≤£∫Ä¯≤°∫¿˚≤1∏vZâÔ7c7jBº´j°∫Yb°∫âj@
6DATA"ºb£∫â±j@º´[°Éudz>ªñ!É∞~Bâç√±Ç!ö±z>ªæ!Çö!ö±æ!Öí!ö±ç€bä!ö±çú≤!è~B—Öï´çŸcë=”/<⁄'ÅLckMÇU≥=Ùàï•çN≤ë°çs≥~Bç;±çÛ±àÉ!ö±••a¿@üí!è≥!ç!Ñ°I£´A¿øI•ç!í≥ë°És≥!ã°ë~Dç;±1c!Åç∑qÉl≥~`!É`!Ü!Ü!Ü!Ü!Ü!Ün!ò!ò`h!É}!ã!å!ì!ëiMJ~`@!Ö!íMJ@
