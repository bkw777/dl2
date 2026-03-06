#!/bin/bash
# Read a .CO file and generate a BASIC loader
# Brian K. White <b.kenyon.w@gmail.com>

LANG=C

: ${FIRST:=0}
: ${LINE_GAP:=1}
: ${LINE_LEN:=256}

# SIGIL, SIGIL+SHIFT, and any UNSAFE+SHIFT must not equal any UNSAFE nor exceed 255.
: ${SHIFT:=64}
: ${SIGIL:='!'}
: ${UNSAFE:=0 1 2 3 4 5 6 7 8 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 34}

CO_IN=$1 ;shift
ACTION=${1^^} ;shift

CO=${CO_IN##*/} ;CO=${CO:0:6} ;CO="${CO%%.*}.CO"

typeset -i i b e SUM TOP END EXE LEN n g
typeset -ia d=()

printf -v e '%u' "'${SIGIL}"
readonly e g=$LINE_GAP u=",${UNSAFE// /,},$e,"
n=$FIRST

abrt () { printf '%s: Usage\n%s IN.CO [call|exec|savem|bsave] > OUT.DO\n%s\n' "$0" "${0##*/}" "$@" >&2 ;exit 1 ; }

# read a binary file into to global int array d[]
ftoi () {
	[[ -r "$1" ]] || abrt "Can't read \"$1\""
	local -i i= ;local x= LANG=C ;d=()
	while IFS= read -d '' -r -n 1 x ;do printf -v d[i++] '%u' "'$x" ;done < $1
}

###############################################################################

# help
[[ "$CO_IN" ]] || abrt

# sanity check SHIFT SIGIL UNSAFE
b=0 ;for i in $UNSAFE $e ;do ((i>b)) && b=$i ;done
((SHIFT>b)) || abrt "SHIFT ($SHIFT) must be higher than the highest UNSAFE ($b)"
(((b+SHIFT)<256)) || abrt "Highest UNSAFE ($b) + SHIFT ($SHIFT) must not exceed 255"

# read the .CO file into d[]
ftoi "$CO_IN"

# parse & discard the .CO header
((TOP=${d[0]}+${d[1]}*256))
((LEN=${d[2]}+${d[3]}*256))
((EXE=${d[4]}+${d[5]}*256))
d=(${d[*]:6})
((LEN==${#d[*]})) || abrt "Corrupt .CO file?\nHeader declares LEN=$LEN\nFile has ${#d[*]} bytes after header"
((END=TOP+LEN-1))
SUM= ;for ((i=0;i<LEN;i++)) { ((SUM+=${d[i]})) ; }

# Stephen Adolh encoding scheme:
# Most bytes just copy input to output without change.
# Unsafe bytes, add a shift value, output sigil and shifted byte.

# loader
printf '%u%c%s - loader: co2ba.sh b.kenyon.w@gmail.com %(%F)T\r' $n "'" "$CO" -1
printf '%uREADF:CLEAR2,F:DEFINTA-E:DEFSNGF-K:DEFSTRL-O:READF,A,J,G,N,E,M:C=0:I=F:H=F+A-1:K=0:D=0:CLS:PRINT"Installing "N\r' $n
printf '%uPRINT@20,USING"###%%";(I-F)*100/A:READL:FORC=1TOLEN(L):O=MID$(L,C,1):IFO=MTHEND=E:NEXT:ELSEB=ASC(O)-D:POKEI,B:D=0:I=I+1:K=K+B:NEXT:IFI<=HTHEN%u\r' $((++n*g)) $n
printf '%uPRINT:IFK<>GTHENPRINT"Bad Checksum":ELSE' $((++n*g))

# action
case "$ACTION" in
	CALL|EXEC) printf '%sJ\r' $ACTION ;;
	SAVEM|BSAVE) printf 'PRINT"Done. Please type: NEW":%sN,F,H,J\r' $ACTION ;;
	*) printf 'PRINT"top "F:PRINT"end "H:PRINT"exe "J\r' ;;
esac

# header
printf '%uDATA%u,%u,%u,%u,"%s",%u,"%c"\r' $((++n*g)) $TOP $LEN $EXE $SUM "$CO" $SHIFT "${SIGIL}"

# data
O= o=
for ((i=0;i<LEN;i++)) {

	((${#O})) || printf -v O '%uDATA"%s' $((++n*g)) "$o"

	b=${d[i]}

	[[ $u = *,${b},* ]] && {
		printf -v o '%03o' $((b+SHIFT))
		printf -v o '%c%b' "${SIGIL}" "\\$o"
	} || {
		printf -v o '%03o' $b
		printf -v o '%b' "\\$o"
	}

	((${#O}+${#o}<LINE_LEN)) && {
		O+=$o
		o=
	} || {
		printf '%s\r' "$O"
		O=
	}
}
((${#O})) && printf '%s\r' "$O"
