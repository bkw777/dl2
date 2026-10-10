0'TEENY - NEC PC-8201A/PC-8300 North America - github.com/bkw777/dl2 - loader: co2ba.sh b.kenyon.w@gmail.com 2026-10-10
0READF:CLEAR12,F:DEFINTA-E,Q,G,K:DEFSNGF,H-J:DEFSTRL-O:READF,A,J,G,N,E,Q:M="!":C=0:I=F:H=F+A-1:K=0:D=0:CLS:?"Installing",N:LOCATE22,0:?"  0%"
1READL:FORC=1TOLEN(L):O=MID$(L,C,1):IFO=MTHEND=E:NEXT:ELSEB=ASC(O)XORD:B=BXORQ:D=0:POKEI,B:I=I+1:K=KXORB:NEXT:LOCATE22,0:?USING"###%";(I-F)*100/A:IFI<=HTHEN1
2IFK<>GTHEN?"Bad Checksum":ELSEM=CHR$(34):L="X.DO":OPENLFOROUTPUTAS1:?#1,"0CLEAR0,"F":EXEC"J":MENU":CLOSE1:?"Please type:":?"KILL"M""L:?"SAVE"M""N:LOADL
3DATA61698,638,61698,31,"TEENY",128,64
4DATA"a@@ybFºa!ö!öbJºjFºπÔrHºaO±•a!ü≥ç!¢Tr(æçÍ!áa!ñ±bæ≥a#¥vbç˝!ççzcz!¢¥æ!ëä!Ö!ñæ!ìä≥≤µç?±ä!ñ±±ñ!ãäB≤}äc≤zHº˜ä(±jBº!Ñ!çj@ºç|,a:≥ç!¢TÉO±y!éxq!é!éaLº7cv@âçË/Vnç!Ä0a1±wç!é\~Gçw≥Ôç7±vZcQ$¥FIç∑qFOv`cEÇ‰±v!Üc0aLº>c∆!ó>!ü!ác˜:äÖ±∆c]Çˇ±o7~E¿!áaJº>ç˛.ö!ñ±cEÇé±aLºç•
5DATA"6çπ±çπ±!èLçπ±MÇ®±ÔpaMº!éc>˜âçì-Ç!ñ±7câ~EÉw≥^A~Aç7±7c3Éz≥ë~Dç7±1c!Åç∑qÉz≥ç€bÇ!ñ±a!ñ±bæ≥çìb•ç™≤jgº!Ö!å) bDºK!ü|C}Ç!É≤!óa˜≤YYYYY>µQ†≤ïc!ûc!ñïc!ûc&+!ûc!ñ )bBº´b@º•çJ,ö!ñ±aHºtçﬂ≤jDº´°zLº!èF@:Û9rLºà˜º‰≤[•aNºI>°7cLÉ∆≤^CçI≤•ï~Cç7±çÔ±Ô!á!èrLºë°â‡ô≤£∫ÄÜ≤°∫¿â≤
6DATA"1∏vZâÔ7c7jBº´j°∫Yb°∫âj@ºb£∫â±j@º´[°Éudz>ªñ!É∞~Bâç?±Ç!ñ±z>ªæ!Çö!ñ±æ!Öí!ñ±ç€bä!ñ±ç™≤!è~B—Öï´çŸcë=”/<⁄'ÅLckMÇc≥=Ùàï•çG≤ë°ç~≥~Bç7±çÔ±à••a¿@üí!ö≥!ç!Ñ°I£´A¿øI•çU≤ë°É~≥!ã°ÉT≤~`!É`!Ü!Ü!Ü!Ü!Ü!Ün!ò!ò`h!É}!ã!å!ì!ëiMJ~`@!Ö!í!íMJ@
