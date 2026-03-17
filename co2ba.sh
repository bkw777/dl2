#!/bin/bash
# Read a .CO file and generate a BASIC loader
# Brian K. White <b.kenyon.w@gmail.com>

LANG=C

: ${COMMENT:=""}
: ${FIRST:=0}
: ${LINE_GAP:=1}
: ${LINE_LEN:=256}
: ${UNSAFE:=0 1 2 3 4 5 6 7 8 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 34}
: ${EDITSAFE:=true}
: ${METHOD:=Y} ;METHOD=${METHOD^^}
: ${EP:='!'}
: ${XA:=^64}       # 0 best ^### +###     default '^64'  
: ${XB:=^128}      # ^### +###            default '^128'
: ${RLE:=false}
: ${RP:=' '}
: ${CHECKSUM:=xor}  # xor, xor+, mod+, sum+
: ${YENC:=false}
: ${CARAT:=false}

# shorthands for some generic standards we can output
$YENC && EP='=' XA="+42" XB="+64"
$CARAT && EP='^' XA=0 XB="+64"
$CARAT && echo "Notice: carat-encoding is broken" >&2
$RLE && echo "RLE ENABLED" >&2

COFN=$1 ;shift
ACTION=${1^^} ;shift
PN=${COFN##*/} ;PN=${PN:0:6} ;PN=${PN%%.*}

typeset -i i b CHK TOP END EXE LEN n g ta tb ep rp rl
typeset -ia d=()

printf -v ep '%u' "'$EP" ;UNSAFE+=" $ep"
$RLE && { printf -v rp '%u' "'$RP" ;UNSAFE+=" $rp" ; }
$EDITSAFE && UNSAFE+=" 127"
readonly g=$LINE_GAP u=",${UNSAFE// /,}," EP ep RP rp
tb=${XB:1} xb=true ;[[ "${XB:0:1}" = "+" ]] && xb=false ;readonly xb tb # transform B, to encode unsafe bytes
unset Ev Qd Qv UNTA
n=$FIRST
TIME=false ;[[ "$ACTION" == "TIME" ]] && TIME=true

abrt () { printf '%s: Usage\n%s IN.CO [call|exec|callba|execba|savem|bsave] > OUT.DO\n%b\n' "$0" "${0##*/}" "$@" >&2 ;exit 1 ; }

# read a binary file into to global int array d[]
ftoi () {
	[[ -r "$1" ]] || abrt "Can't read \"$1\""
	local -i i= ;local x= LANG=C ;d=()
	while IFS= read -d '' -r -n 1 x ;do printf -v d[i++] '%u' "'$x" ;done < $1
}

# $XA -> $xa $ta
find_xa () {
	xa=true ta=${XA:1}
	case "$XA" in
		\+*) xa=false ;;
		best)
			echo "trying all possible XA values..." >&2
			local -i i n t s=$((LEN*2))
			for ((n=0;n<255;n++)) {

				# xor
				for ((i=0,t=LEN;i<LEN;i++)) { [[ $u = *,$((d[i]^n)),* ]] && ((t++)) ; }
				((t<s)) && s=$t ta=$n XA="^$n"
				#echo "^$n -> $t" >&2

				# rot
				for ((i=0,t=LEN;i<LEN;i++)) { [[ $u = *,$(((d[i]+n)%256)),* ]] && ((t++)) ; }
				((t<s)) && xa=false s=$t ta=$n XA="+$n"
				#echo "+$n -> $t" >&2

				:
			}
			#echo "XA=$XA  ->  $s bytes" >&2
			echo "XA=$XA" >&2
			;;
	esac
	readonly xa ta
}

# encode a single byte $1 to $o
enc_o () {
	local -i b=$1
	((ta)) && { $xa && b=$((b^ta)) || b=$(((b+ta)%256)) ; }
	[[ $u = *,$b,* ]] && {
		$xb && b=$((b^tb)) || b=$(((b+tb)%256))
		printf -v o '%03o' $b
		printf -v o '%c%b' "$EP" "\\$o"
	} || {
		printf -v o '%03o' $b
		printf -v o '%b' "\\$o"
	}
}

# write a run-length to $o
# $1 byte value
# $2 number of copies
rle_o () {
	local -i b=$1 n=$2 ;local x=
	#echo "b=$b n=$n" >&2
	((n>2)) && {
		enc_o $n
		o="$RP$o"
	} || {
		enc_o $b ;x="$o" o=
		for ((;n;n--)) { o+="$x" ; }
	}
}

###############################################################################

# help
[[ "$COFN" ]] || abrt

# read the .CO file into d[]
ftoi "$COFN"

# parse & discard the .CO header
((TOP=${d[0]}+${d[1]}*256))
((LEN=${d[2]}+${d[3]}*256))
((EXE=${d[4]}+${d[5]}*256))
d=(${d[*]:6})
((LEN==${#d[*]})) || abrt "Corrupt .CO file?\nHeader declares LEN=$LEN\nFile has ${#d[*]} bytes after header"
((END=TOP+LEN-1))

# checksum - a few different methods
# M=256 min, M=32512 max without exceeding INT. n%32512 -> 32511+1+255=32767
CHK=0 ik=true
case "$CHECKSUM" in
	sum\+|strongest) for ((i=0;i<LEN;i++)) { ((CHK+=d[i]+1)) ; } ;K="K+B+1" ik=false ;; # SNG - 144 seconds
	mod\+|stronger) M=32512 ;for ((i=0;i<LEN;i++)) { ((CHK=(CHK+1+d[i])%M)) ; } ;K="(K+B+1)MOD$M" ;; # INT - 142 seconds (any M)
	xor\+|strong) for ((i=0;i<LEN;i++)) { ((CHK^=d[i]+1)) ; } ;K="KXORB+1" ik=false ;; # SNG - 134 seconds   DEFSNG because while CHK generally stays small it is possible to exceed INT with the right input data
	xor|fast|*) for ((i=0;i<LEN;i++)) { ((CHK^=d[i])) ; } ;K="KXORB" ;; # INT - 126 seconds
esac

# loader
printf -v O '%u%c%s%%s - loader: co2ba.sh b.kenyon.w@gmail.com %(%F)T' $n "'" "$PN" -1
x=${COMMENT:+ - $COMMENT} ;((i=255-${#O}+2)) ;((${#x}>i)) && x=${x:0:i}
printf '%s\r' "${O/\%s/$x}"

$TIME && tn=20
$ik && di=",G,K" dn="F,H-J" || di="" dn="F-K"  # DEFINT DEFSNG
case ${METHOD^^} in
	Y) # !yenc - Adolph/B9/White yenc-like encoding
		unset Qd Qv UNTA ;find_xa ;((ta)) && { Qd=",Q" ;$xa && Qv=$ta UNTA="B=BXORQ:" || Qv=$((256-ta)) UNTA="B=(B+Q)MOD256:" ; }
		$xb && Ev=$tb UNTB="ASC(O)XORD" || Ev=$((256-tb)) UNTB="(ASC(O)+D)MOD256"

		$RLE && {
			K=${K/B/P}

			printf '%uREADF:CLEAR12,F:DEFINTA-E,P,S%s%s:DEFSNG%s:DEFSTRL-O,R:READF,A,J,G,N,E%s:M="%c":C=0:I=F:H=F+A-1:K=0:D=0:R="%c":S=0:B=-1:P=-1:CLS:?USING"Installing \    \   0%%";N\r' $n "$Qd" "$di" "$dn" "$Qd" "$EP" "$RP"
			$TIME && printf '%uGOSUB%u\r' $((++n*g)) $((tn*g))
			printf '%uREADL:FORC=1TOLEN(L):O=MID$(L,C,1):IF(O=M)THEND=E:NEXT:ELSEIF(O=R)THENS=1:NEXT\r' $((++n*g)) ;((l=n))

			# fastest
			printf '%uB=%s:%sD=0:IFS=0THENP=B:POKEI,P:I=I+1:K=%s:NEXT:ELSEFORS=-BTO-1:POKEI,P:I=I+1:K=%s:NEXT:NEXT\r' $((++n*g)) "$UNTB" "$UNTA" "$K" "$K"
			printf '%u?@18,USING"###%%";(I-F)*100/A:IFI<=HTHEN%u\r' $((++n*g)) $((l*g))

			# slower
			#printf '%uB=%s:%sD=0:IFS=0THENP=B:POKEI,P:I=I+1:K=%s:ELSEFORS=-BTO-1:POKEI,P:I=I+1:K=%s:NEXT\r' $((++n*g)) "$UNTB" "$UNTA" "$K" "$K"
			#printf '%uNEXT:?@18,USING"###%%";(I-F)*100/A:IFI<=HTHEN%u\r' $((++n*g)) $((l*g))

			# slowest
			#printf '%uB=%s:%sD=0:IFS=0THENP=B:B=1\r' $((++n*g)) "$UNTB" "$UNTA"
			#printf '%uFORS=-BTO-1:POKEI,P:I=I+1:K=%s:NEXT:NEXT:?@18,USING"###%%";(I-F)*100/A:IFI<=HTHEN%u\r' $((++n*g)) "$K" $((l*g))

		} || {
			printf '%uREADF:CLEAR12,F:DEFINTA-E%s%s:DEFSNG%s:DEFSTRL-O:READF,A,J,G,N,E%s:M="%c":C=0:I=F:H=F+A-1:K=0:D=0:CLS:?"Installing "N"   0%%"\r' $n "$Qd" "$di" "$dn" "$Qd" "$EP"
			$TIME && printf '%uGOSUB%u\r' $((++n*g)) $((tn*g))
			printf '%uREADL:FORC=1TOLEN(L):O=MID$(L,C,1):IFO=MTHEND=E:NEXT:ELSEB=%s:%sD=0:POKEI,B:I=I+1:K=%s:NEXT:?@18,USING"###%%";(I-F)*100/A:IFI<=HTHEN%u\r' $((++n*g)) "$UNTB" "$UNTA" "$K" $((n*g))
		}
	;;
	B) # Same as Y but avoids using IF in the inner loop, but actually runs slower - NO RLE
		unset Qd Qv UNTA ;find_xa ;((ta)) && { Qd=",Q" ;$xa && Qv=$ta UNTA="B=BXORQ:" || Qv=$((256-ta)) UNTA="B=(B+Q)MOD256:" ; }
		$xb && Ev=$tb UNTB="BXORE*D" || Ev=$((256-tb)) UNTB="(B+E*D)MOD256"
		printf '%uREADF:CLEAR12,F:DEFINTA-E,O-P%s%s:DEFSNG%s:DEFSTRL-N:READF,A,J,G,N,E%s:M="":C=0:I=F:H=F+A-1:K=0:D=0:O=%u:P=0:CLS:?"Installing "N"   0%%"\r' $n "$Qd" "$di" "$dn" "$Qd" $ep
		$TIME && printf '%uGOSUB%u\r' $((++n*g)) $((tn*g))
		K="${K/B/B\*P}" K="${K/1/P}"
		# P=SGN(BXORO) is faster than P=-(P<>O)
		printf '%uREADL:FORC=1TOLEN(L):B=ASC(MID$(L,C,1)):P=SGN(BXORO):B=%s:%sD=PXOR1:POKEI,B:I=I+P:K=%s:NEXT:?@18,USING"###%%";(I-F)*100/A:IFI<=HTHEN%u\r' $((++n*g)) "$UNTB" "$UNTA" "$K" $((n*g))
	;;
	H) # hex pairs
		typeset -ra h=({a..p})  # hex data output alphabet
		printf -v Ev '%u' "'${h[0]}"
		printf '%uREADF:CLEAR12,F:DEFINTA-E%s:DEFSNG%s:DEFSTRL-N:READF,A,J,G,N,E:M="":C=0:I=F:H=F+A-1:K=0:CLS:?"Installing "N"   0%%";\r' $n "$di" "$dn"
		$TIME && printf '%uGOSUB%u\r' $((++n*g)) $((tn*g))
		printf '%uREADL:FORC=1TOLEN(L)STEP2:B=(ASC(MID$(L,C,1))-E)*16+ASC(MID$(L,C+1,1))-E:POKEI,B:I=I+1:K=%s:NEXT:?@18,USING"###%%";(I-F)*100/A:IFI<=HTHEN%u\r' $((++n*g)) "$K" $((n*g))
	;;
	I) # ints
		$TIME && {
			printf '%uREADF:CLEAR16,F:DEFINTA-E%s:DEFSNG%s:DEFSTRL-N:READF,A,J,G,N:H=F+A-1:K=0:CLS:?"Installing "N\r' $n "$di" "$dn"
			printf '%uGOSUB%u\r' $((++n*g)) $((tn*g))
			printf '%uFORI=FTOH:READB:POKEI,B:K=%s:?".";:NEXT:?\r' $((++n*g)) "$K"
		} || {
			printf '%uREADF:CLEAR12,F:DEFINTA-E%s:DEFSNG%s:DEFSTRL-N:READF,A,J,G,N:H=F+A-1:K=0:CLS:?"Installing "N:FORI=FTOH:READB:POKEI,B:K=%s:?".";:NEXT:?\r' $n "$di" "$dn" "$K"
		}

	;;
esac

printf '%uIFK<>GTHEN?"Bad Checksum":ELSE' $((++n*g))

# action
case "$ACTION" in
	TIME) printf 'Y=Z:GOSUB%u:?Z-Y"seconds"\r' $((tn*g)) ;;
	CALL|EXEC) printf '%sJ\r' $ACTION ;;
	SAVEM|BSAVE) printf '?"Please type: NEW":%sN,F,H,J\r' $ACTION ;;
	CALLBA|EXECBA) printf 'M=CHR$(34):L="X.DO":OPENLFOROUTPUTAS1:?#1,"0CLEAR0,"F":%s"J:CLOSE1:?"Please type:":?"KILL"M""L:?"SAVE"M""N:LOADL\r' ${ACTION:0:4} ;;
	*) printf '?"top "F:?"end "H:?"exe "J\r' ;;
esac

$TIME && printf '%uEND\r%uL=TIME$:Z=VAL(LEFT$(L,2))*60*60+VAL(MID$(L,4,2))*60+VAL(RIGHT$(L,2)):RETURN\r' $((++n*g)) $(((n=tn)*g))

# header
printf '%uDATA%u,%u,%u,%u,"%s"%s%s\r' $((++n*g)) $TOP $LEN $EXE $CHK "$PN" "${Ev:+,$Ev}" "${Qv:+,$Qv}"

# data
O= o= rl=0 pb=-1
for ((i=0;i<LEN;i++)) {

	((${#O})) || {
		O="$((++n*g))DATA" 
		case $METHOD in
			H|I) ;;
			*) O+='"' ;;
		esac
		O+="$o"
	}

	((cb=d[i]))

	case $METHOD in
		I) o=$cb, ;;
		H) o=${h[cb/16]}${h[cb%16]} ;;
		*)
			$RLE && ((cb==pb && rl++<255)) || {
				enc_o $cb
				((rl)) && { x="$o" ;rle_o $pb $rl ;rl=0 o+="$x" ; }
			}
			;;
	esac

	((pb=cb))

	((${#O}+${#o}<LINE_LEN)) && {
		O+=$o o=
	} || {
		[[ "$METHOD" == "I" ]] && O=${O:0:-1}
		printf '%s\r' "$O" ;O=
	}

}

((${#O})) && {
	[[ "$METHOD" == "I" ]] && O=${O:0:-1}
	((rl)) && { rle_o $pb $rl ;O+="$o" ; }
	printf '%s\r' "$O"
}
