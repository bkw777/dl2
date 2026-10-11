0'TEENY - NEC PC-8201A/PC-8300 North America - github.com/bkw777/dl2 - loader: co2ba.sh b.kenyon.w@gmail.com 2026-10-10
0READF:CLEAR12,F:DEFINTA-E,Q,G,K:DEFSNGF,H-J:DEFSTRL-O:READF,A,J,G,N,E,Q:M="!":C=0:I=F:H=F+A-1:K=0:D=0:CLS:?"Installing",N:LOCATE22,0:?"  0%"
1READL:FORC=1TOLEN(L):O=MID$(L,C,1):IFO=MTHEND=E:NEXT:ELSEB=ASC(O)XORD:B=BXORQ:D=0:POKEI,B:I=I+1:K=KXORB:NEXT:LOCATE22,0:?USING"###%";(I-F)*100/A:IFI<=HTHEN1
2IFK<>GTHEN?"Bad Checksum":ELSEM=CHR$(34):L="X.DO":OPENLFOROUTPUTAS1:?#1,"0CLEAR0,"F":EXEC"J":MENU":CLOSE1:?"Please type:":?"KILL"M""L:?"SAVE"M""N:LOADL
3DATA61705,631,61705,216,"TEENY",128,64
4DATA"a@@ybFºa!ö!öbJºjFºπÔrHºaV±•a ≥ç!¢Tr(æçÍ!áa!ù±bæ≥a#¥vbç˝!ççzcz!¢¥æ!ëä!Ö!ñæ!ìä•≤µç∆±ä!ù±±ñ!ãäI≤}ä[≤zHº˜ä/±jBº!Ñ!çj@ºç|,a;≥ç!¢TÉV±y!éxq!é!éaLº7cv@âçË/Vnç!Ä0a8±wç!é\~Gçi≥Ôç>±vZcQ$¥FIç∑qFOv`cEÇÎ±v!Üc0aLº>c∆!ó>!ü!ác˜:äå±∆c]ÇÜ±o7~E¿!áaJº>ç˛.ö!ù±cEÇï±aLºç•6
5DATA"ç@≤ç@≤!èLç@≤MÇØ±ÔpaMº!éc>˜âçì-Ç!ù±7câ~EÉi≥^A~Aç>±7c3Él≥ç€bÇ!ù±çìb•çú≤jgº!Ö!å) bDºK!ü|C}Çu≤!óaÈ≤YYYYY>µQí≤ïc!ûc!ñïc!ûc&+!ûc!ñ )bBº´b@º•çJ,ö!ù±aHºtç—≤jDº´°zLº!èF@:Û9rLºà˜º÷≤[•aNºI>°7cLÉ8≤^CçP≤•ï~Cç>±çˆ±Ô!á!èrLºë°â‡ã≤£∫Ä¯≤°∫¿˚≤1∏vZâÔ7c7jBº´j°∫Yb°∫âj@ºb
6DATA"£∫â±j@º´[°Éudz>ªñ!É∞~Bâç∆±Ç!ù±z>ªæ!Çö!ù±æ!Öí!ù±ç€bä!ù±çú≤!è~B—Öï´çŸcë=”/<⁄'ÅLckMÇU≥=Ùàï•çN≤ë°çs≥~Bç>±çˆ±àÉ!ù±••a¿@üí!è≥!ç!Ñ°I£´A¿øI•ç!í≥ë°És≥!ã°ë~Dç>±1c!Åç∑qÉl≥~`!É`!Ü!Ü!Ü!Ü!Ü!Ün!ò!ò`h!É}!ã!å!ì!ëiMJ~`@!Ö!íMJ@
