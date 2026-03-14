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
: ${METHOD:=Y}
: ${EP:='!'}
: ${XA:=^64}
: ${XB:=^128}
: ${YENC:=false}
: ${CARAT:=false}

: ${RLE:=false}
: ${RP:=' '}

# shorthands for some generic standards we can output
$YENC && EP='=' XA="+42" XB="+64"
$CARAT && EP='^' XA=0 XB="+64"
$CARAT && echo "Notice: carat-encoding is broken" >&2
$RLE && echo "(UG)RLE ENABLED  Notice: WIP: currently only works if XA=0, and only supported in METHOD=Y" >&2

COFN=$1 ;shift
ACTION=${1^^} ;shift
PN=${COFN##*/} ;PN=${PN:0:6} ;PN=${PN%%.*}

typeset -i i b CHK TOP END EXE LEN n g ta tb ep rp rl
typeset -ia d=()

printf -v ep '%u' "'$EP" ;UNSAFE+=" $ep"
$RLE && { printf -v rp '%u' "'$RP" ;UNSAFE+=" $rp" ; }
$EDITSAFE && UNSAFE+=" 127"
readonly g=$LINE_GAP u=",${UNSAFE// /,}," EP ep RP rp
#ta=${XA:1} xa=true ;[[ "${XA:0:1}" = "+" ]] && xa=false ;readonly xa ta # transform A, to shift all bytes
tb=${XB:1} xb=true ;[[ "${XB:0:1}" = "+" ]] && xb=false ;readonly xb tb # transform B, to encode unsafe bytes
unset Ev Qd Qv
n=$FIRST

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

# Transform every byte according to XA
# Write the BASIC code to reverse it in UNTA
transform_a () {
	find_xa
	unset Qd Qv UNTA
	((ta)) || return
	$xa && {
		# xor
		for ((i=0;i<LEN;i++)) { ((d[i]^=ta)) ; }
		Qd=",Q" Qv=$ta
		UNTA="B=BXORQ:"
	} || {
		# rot
		for ((i=0;i<LEN;i++)) { ((d[i]=(d[i]+ta)%256)) ; }
		Qd=",Q" Qv=$((256-ta))
		UNTA="B=(B+Q)MOD256:"
	}
}

# encode a single byte $1 to $o
enc_o () {
	local -i b=$1
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
	((n>2)) && {
		x="$RP"
		enc_o $n
		x+="$o"
	} || {
		for ((;n;n--)) {
			enc_o $b
			x+="$o"
		}
	}
	o="$x"
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

# rolling + incrementing xor checksum
CHK=0 ;for ((i=0;i<LEN;i++)) { ((CHK=(CHK^d[i])+1)) ; }

# loader

printf -v O '%u%c%s%%s - loader: co2ba.sh b.kenyon.w@gmail.com %(%F)T' $n "'" "$PN" -1
x=${COMMENT:+ - $COMMENT} ;((i=255-${#O}+2)) ;((${#x}>i)) && x=${x:0:i}
printf '%s\r' "${O/\%s/$x}"

case $METHOD in
	Y) # !yenc - Adolph/B9/White yenc-like encoding
		transform_a
		$xb && Ev=$tb UNTB="ASC(O)XORD" || Ev=$((256-tb)) UNTB="(ASC(O)+D)MOD256"

		$RLE && { # works!!!! if XA=0, otherwise bad checksum - but even then call to the exe addr seems to work anyway
			printf '%uREADF:CLEAR8,F:DEFINTA-E,G,K,P,S,T%s:DEFSNGF,H-J:DEFSTRL-O,R:READF,A,J,G,N,E%s:M="%c":C=0:I=F:H=F+A-1:K=0:D=0:R="%c":S=0:B=-1:P=-1:CLS:?USING"Installing \    \   0%%";N\r' $n "$Qd" "$Qd" "$EP" "$RP"
			printf '%uREADL:FORC=1TOLEN(L):O=MID$(L,C,1):IF(O=M)THEND=E:NEXT\r' $((++n*g)) ;((l=n))
			printf '%uIF(D=0ANDO=R)THENS=1:NEXT\r' $((++n*g))
			printf '%uB=%s:%sD=0\r' $((++n*g)) "$UNTB" "$UNTA"
			printf '%uIFS>0THENFORT=1TOB:POKEI,P:I=I+1:K=(KXORP)+1:NEXTT\r' $((++n*g))
			printf '%uIFS=0THENP=B:POKEI,P:I=I+1:K=(KXORP)+1\r' $((++n*g))
			printf '%uS=0:NEXT:?@18,USING"###%%";(I-F)*100/A:IFI<=HTHEN%u\r' $((++n*g)) $((l*g))
		} || {
			printf '%uREADF:CLEAR2,F:DEFINTA-E,G,K%s:DEFSNGF,H-J:DEFSTRL-O:READF,A,J,G,N,E%s:M="%c":C=0:I=F:H=F+A-1:K=0:D=0:CLS:?"Installing "N"   0%%"\r' $n "$Qd" "$Qd" "$EP"
			printf '%uREADL:FORC=1TOLEN(L):O=MID$(L,C,1):IFO=MTHEND=E:NEXT:ELSEB=%s:%sD=0:POKEI,B:I=I+1:K=(KXORB)+1:NEXT:?@18,USING"###%%";(I-F)*100/A:IFI<=HTHEN%u\r' $((++n*g)) "$UNTB" "$UNTA" $((n*g))
		}

	;;
	B) # Same as Y but avoids using IF in the inner loop, but actually runs slower
		transform_a
		$xb && Ev=$tb UNTB="BXORE*D" || Ev=$((256-tb)) UNTB="(B+E*D)MOD256"
		printf '%uREADF:CLEAR2,F:DEFINTA-E,G,K,O-P%s:DEFSNGF,H-J:DEFSTRL-N:READF,A,J,G,N,E%s:M="":C=0:I=F:H=F+A-1:K=0:D=0:O=%u:P=0:CLS:?"Installing "N"   0%%"\r' $n "$Qd" "$Qd" $ep
		# X=SGN(AXORB) could be X=-(A<>B)  but SGN(XOR) is slightly faster, 40 vs 43 seconds for 10000
		printf '%uREADL:FORC=1TOLEN(L):B=ASC(MID$(L,C,1)):P=SGN(BXORO):B=%s:%sPOKEI,B:I=I+P:K=((KXORB)+1)*P:D=PXOR1:NEXT:?@18,USING"###%%";(I-F)*100/A:IFI<=HTHEN%u\r' $((++n*g)) "$UNTB" "$UNTA" $((n*g))
	;;
	H) # hex pairs
		typeset -ra h=({a..p})  # hex data output alphabet
		printf -v Ev '%u' "'${h[0]}"
		printf '%uREADF:CLEAR2,F:DEFINTA-E,G,K:DEFSNGF,H-J:DEFSTRL-N:READF,A,J,G,N,E:M="":C=0:I=F:H=F+A-1:K=0:CLS:?"Installing "N"   0%%";\r' $n
		printf '%uREADL:FORC=1TOLEN(L)STEP2:B=(ASC(MID$(L,C,1))-E)*16+ASC(MID$(L,C+1,1))-E:POKEI,B:I=I+1:K=(KXORB)+1:NEXT:?@18,USING"###%%";(I-F)*100/A:IFI<=HTHEN%u\r' $((++n*g)) $((n*g))
	;;
	I) # ints
		printf '%uREADF:CLEAR2,F:DEFINTA-E,G,K:DEFSNGF,H-J:DEFSTRL-N:READF,A,J,G,N:H=F+A-1:K=0:CLS:?"Installing "N:FORI=FTOH:READB:POKEI,B:K=(KXORB)+1:?".";:NEXT:?\r' $n
	;;
esac

printf '%uIFK<>GTHEN?"Bad Checksum":ELSE' $((++n*g))

# action
case "$ACTION" in
	CALL|EXEC) printf '%sJ\r' $ACTION ;;
	SAVEM|BSAVE) printf '?"Please type: NEW":%sN,F,H,J\r' $ACTION ;;
	CALLBA|EXECBA) printf 'M=CHR$(34):L="X.DO":OPENLFOROUTPUTAS1:?#1,"0CLEAR0,"F":%s"J:CLOSE1:?"Please type:":?"KILL"M""L:?"SAVE"M""N:LOADL\r' ${ACTION:0:4} ;;
	*) printf '?"top "F:?"end "H:?"exe "J\r' ;;
esac

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
			$RLE && ((cb==pb && rl<255)) && {
				: $((rl++))
			} || {
				x=
				$RLE && { rle_o $pb $rl ;rl=0 x="$o" ; }
				enc_o $cb
				o="$x$o"
			}
			;;
	esac

	((pb=cb))

	((${#O}+${#o}<LINE_LEN)) && {
		O+=$o
		o=
	} || {
		[[ "$METHOD" == "I" ]] && O=${O:0:-1}
		printf '%s\r' "$O"
		O=
	}

}

((${#O})) && {
	[[ "$METHOD" == "I" ]] && O=${O:0:-1}
	$RLE && { rle_o $pb $rl ;O+="$o" ; }
	printf '%s\r' "$O"
}

# output trailing ^Z for directly catting into BASIC without dl -b
#printf '%b' '\032'

exit

# paste this function into a terminal for a quick & dirty bootstrapper
#   $ RLE=true XA=0 co2ba.sh ALTERN.CO callba >t
#   $ tsend t
tsend () {
	local d=${2:-/dev/ttyUSB0} b=${3:-9600} s=([19200]=9 [9600]=8 [4800]=7 [2400]=6 [1200]=5 [600]=4 [300]=3)
	((${#1})) || { echo "${FUNCNAME[0]} FILE.DO [/dev/ttyX] [baud]" ;return ; }
	[[ -c $d ]] || { echo /dev/tty* ;return ; }
	((${#s[b]})) || { echo ${!s[*]} ;return ; }
	echo "NEC: RUN\"COM:${s[b]}N81XN"
	echo "100/102/200/K85/M10: RUN\"COM:${s[b]}8N1ENN"
	read -p "Press [Enter] whean ready: " ;echo "Sending..."
	stty -F $d $b raw pass8 clocal cread time 1 min 1 -crtscts ixon ixoff flusho -drain
	cat $1 >$d
	printf '%b' '\x1A' >$d
}
