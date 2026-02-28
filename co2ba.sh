#!/bin/bash
# Read a .CO file and generate a BASIC loader
# Brian K. White <b.kenyon.w@gmail.com>

BYTES_PER_DATA_LINE=120

CO_IN=$1 ;shift
ACTION=${1^^} ;shift

CO=${CO_IN##*/} ;CO=${CO:0:6} ;CO="${CO%%.*}.CO"

typeset -ra h=({a..p})  # hex data output alphabet
typeset -i i t b c SUM TOP END EXE LEN LINE=0
typeset -a d=()

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

# BASIC loader
printf '0%c%s - loader: co2ba.sh b.kenyon.w@gmail.com %(%F)T\r' "'" "$CO" -1
printf '0CLEAR0,%u:A=%u:S=0:N$="%s":CLS:?"Installing "N$" ...";\r' $TOP $TOP "$CO"
printf '%uD$="":READD$:FORI=1TOLEN(D$)STEP2:B=(ASC(MID$(D$,I,1))-%u)*16+ASC(MID$(D$,I+1,1))-%u:POKEA,B:A=A+1:S=S+B:NEXT:?".";:IFA<%uTHEN%u\r' $((++LINE)) "'${h[0]}" "'${h[0]}" $((END+1)) $LINE
printf '%uIFS<>%uTHEN?"Bad Checksum":END\r' $((++LINE)) $SUM

# action after loading
case "$ACTION" in
	CALL|EXEC) printf '%u%s%u\r' $((++LINE)) $ACTION $EXE ;;
	SAVEM|BSAVE) printf '%u?:?"Done. Please type: NEW":%sN$,%u,%u,%u\r' $((++LINE)) $ACTION $TOP $END $EXE ;;
	*) printf '%uCLS:?"Loaded:":?"top %u":?"end %u":?"exe %u":?"Please type: NEW"\r' $((++LINE)) $TOP $END $EXE ;;
esac

# DATA lines
c= ;for ((i=0;i<LEN;i++)) {
	((c++)) || printf '%uDATA' $((++LINE))
	printf '%c%c' ${h[${d[i]}/16]} ${h[${d[i]}%16]}
	((c<BYTES_PER_DATA_LINE)) || { c=0 ;printf '\r' ; }
}
((c)) && printf '\r'
