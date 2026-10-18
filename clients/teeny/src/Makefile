# Makefile for TEENY
# Requires https://github.com/z88dk/z88dk
# Requires https://github.com/bkw777/dl2

NAME = TEENY
SRC = $(NAME).S85
MODELS = 100na K85na M10na M10eu 200na NECna
URL = github.com/bkw777/dl2
CO2BA_CMD = callba      # savem / callbe
CO2BA_CMD_NEC = execba  # bsave / execba
#ASFLAGS := -v
DEFAULT_TARGET = DO

AS = z88dk-z80asm
af = -m8085_strict -no-synth # -v
# Adds -DZ88DK=ver to flags if the assembler is z88dk.
af += $(shell $(AS) |awk '/^Z80 Macro Assembler /{split($$4,a,"-");if (a[1]>0) print "-DZ88DK="a[1]}')

# -DBAUD=baud   # [19200] 9600 4800 2400 1200 600 300 110 75
# -DDSR         # include code for DSR checks
# -DERRCODES    # add code for different error codes for different conditions
# -DROM         # don't use any self-modifying code
# -ORG=addr     # generate a binary with specified ORG instead of HIMEM-LENGTH
# -DHIMEM=addr  # generate a binary with specified END address instead of MAXRAM

# build all *.CO and *.DO with default config
#   $ make clean all
# build all *.CO and *.DO with custom config
#   $ make clean all DBAUD=9600 DSR=true ERRCODES=true
# build a rom-embeddable *.bin for Olivetti M-10 EU
#   $ make clean M10eu ROM=true ORG=0x7CFB

ifdef BAUD
	af += -DBAUD=$(BAUD)
endif
ifeq ($(DSR),true)
	af += -DDSR
endif
ifeq ($(ERRCODES),true)
	af += -DERRCODES
endif
ifeq ($(ROM),true)
	af += -DROM
	DEFAULT_TARGET = bin
endif
ifdef ORG
	af += -DORG=$(ORG)
endif
ifdef HIMEM
	af += -DHIMEM=$(HIMEM)
endif

ASFLAGS += $(af)

# don't delete .CO when building .DO
.SECONDARY:

.PHONY: all
all: $(MODELS)

.PHONY: $(MODELS)
$(MODELS): % : $(NAME)_%.$(DEFAULT_TARGET)

$(patsubst %,$(NAME)_%.CO,$(MODELS)): $(NAME)_%.CO : $(SRC) %.S85
	@$(AS) $(ASFLAGS) -DMODEL_$* -b -o=$@ $< && \
	CMD="$(AS) $(ASFLAGS) -DMODEL_$* -DPRGLEN=`wc -c <$@` -b -o=$@ $<" ; \
	echo "$$CMD" ; \
	eval $$CMD
	@rm *.o
	@case $* in 200*|NEC*) echo '!NOTICE!  $* does not work. See ../orig/disasm for working $* source' ;; esac

%.bin %.rom: %.CO
	@mv -v $< $@

%.DO: %.CO
	@which -s co2ba || echo "Please install https://github.com/bkw777/dl2 for co2ba to generate $@"
	@case "$*" in *NEC*) o=$(CO2BA_CMD_NEC) ;; *) o=$(CO2BA_CMD) ;; esac ; \
	x="$*" ;while read a b c ;do [ "_$$b" = "_platform:" ] && break || c= ;done < $${x##*_}.S85 ; \
	CMD="PN=$(NAME) COMMENT=\"$$c - $(URL)\" co2ba $< $$o >$@" ; \
	echo "$$CMD" ; \
	eval $$CMD

.PHONY: clean
clean:
	rm -f *.o *.bin *.sym *.lis *.map *.def *.CO *.DO *.rom
