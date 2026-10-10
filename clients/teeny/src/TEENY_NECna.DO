0'TEENY - NEC PC-8201A/PC-8300 North America - github.com/bkw777/dl2 - loader: co2ba.sh b.kenyon.w@gmail.com 2026-10-10
0READF:CLEAR12,F:DEFINTA-E,Q,G,K:DEFSNGF,H-J:DEFSTRL-O:READF,A,J,G,N,E,Q:M="!":C=0:I=F:H=F+A-1:K=0:D=0:CLS:?"Installing",N:LOCATE22,0:?"  0%"
1READL:FORC=1TOLEN(L):O=MID$(L,C,1):IFO=MTHEND=E:NEXT:ELSEB=ASC(O)XORD:B=BXORQ:D=0:POKEI,B:I=I+1:K=KXORB:NEXT:LOCATE22,0:?USING"###%";(I-F)*100/A:IFI<=HTHEN1
2IFK<>GTHEN?"Bad Checksum":ELSEM=CHR$(34):L="X.DO":OPENLFOROUTPUTAS1:?#1,"0CLEAR0,"F":EXEC"J":MENU":CLOSE1:?"Please type:":?"KILL"M""L:?"SAVE"M""N:LOADL
3DATA61698,638,61698,196,"TEENY",128,64
4DATA"a@@ybFºa!ö!öbJºjFºπÔrHºaO±•a!ü≥ç!¢Tr(æçÍ!áa!ñ±bæ≥a#¥vbç˝!ççzcz!¢¥æ!ëä!Ö!ñæ!ìä≥≤µçó±ä!ñ±±ñ!ãäí±}äc≤zHº˜ä(±jBº!Ñ!çj@ºç|,a:≥ç!¢TÉO±y!éxq!é!éaLº7cv@âaLº>c∆!ó>!ü!ác˜:ä’±∆c]Çœ±o7~E¿!áaJº>ç˛.ö!ñ±cEÇﬁ±aLºç•6çâ±çâ±!èLçâ±MÇ¯±ÔpaMº!éc>˜âçì-Ç!ñ±7câ~EÉw≥çË/Vnç!Ä
5DATA"0a1±wç!é\~Gçw≥Ôç7±vZcQ$¥FIç∑qFOv`cEÇº±v!Üc0^A~Aç7±7c3Éz≥ë~Dç7±1c!Åç∑qÉz≥ç€bÇ!ñ±a!ñ±bæ≥çìb•ç™≤jgº!Ö!å) bDºK!ü|C}Ç!É≤!óa˜≤YYYYY>µQ†≤ïc!ûc!ñïc!ûc&+!ûc!ñ )bBº´b@º•çJ,ö!ñ±aHºtçﬂ≤jDº´°zLº!èF@:Û9rLºà˜º‰≤[•aNºI>°7cLÉ∆≤^CçI≤•ï~Cç7±ç?±Ô!á!èrLºë°â‡ô≤£∫ÄÜ≤°∫¿â≤
6DATA"1∏vZâÔ7c7jBº´j°∫Yb°∫âj@ºb£∫â±j@º´[°Éudz>ªñ!É∞~Bâçó±Ç!ñ±z>ªæ!Çö!ñ±æ!Öí!ñ±ç€bä!ñ±ç™≤!è~B—Öï´çŸcë=”/<⁄'ÅLckMÇc≥=Ùàï•çG≤ë°ç~≥~Bç7±ç?±à••a¿@üí!ö≥!ç!Ñ°I£´A¿øI•çU≤ë°É~≥!ã°ÉT≤~`!É`!Ü!Ü!Ü!Ü!Ü!Ün!ò!ò`h!É}!ã!å!ì!ëiMJ~`@!Ö!í!íMJ@
