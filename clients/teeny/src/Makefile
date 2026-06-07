# Makefile for TEENY
# Requires https://github.com/z88dk/z88dk
# Requires https://github.com/bkw777/dl2

MODELS = 100 K85 M10 # 200 NEC
NAME = TNY
SRC = TEENY.S85

AS = z88dk-z80asm
ASFLAGS := -m8085 -no-synth # -v
# z88dk-z80asm <= 24602 has a bug that it only accepts the zilog syntax for RST.
# If $(AS) is z88dk then add -DZ88DK=ver to flags. Used in m100_common.S85
# fixed as of https://github.com/z88dk/z88dk/commit/db5c165
ASFLAGS += $(shell $(AS) |awk '/^Z80 Macro Assembler /{split($$4,a,"-");if (a[1]>0) print "-DZ88DK="a[1]}')

# -DHIMEM=addr         # generate a relocated binary with specified HIMEM/END address
XFLAGS :=

# don't delete .CO when building .DO etc.
.SECONDARY:

.PHONY: all
all: $(MODELS)

.PHONY: $(MODELS)
$(MODELS): % : $(NAME)%.DO

# We need the length of the binary to set ORG, but we don't know the length
# until after compile. So we compile once with default org 0 and no CO header
# just to get the length. Then compile again with correct ORG & CO header.
# Also generate map file (-m) to provide the function call addresses,
# and filter .map to produce .calls
$(patsubst %,$(NAME)%.CO,$(MODELS)): $(NAME)%.CO : $(SRC) %.S85
	$(AS) $(ASFLAGS) $(XFLAGS) -DMODEL_$* -b -o=$@ $< && \
	n=$$(wc -c <$@) && \
	$(AS) $(ASFLAGS) $(XFLAGS) -DMODEL_$* -DPRGLEN=$$n -b -o=$@ $<
	@rm *.o
	@if test "$*" = "K85" ;then echo '!NOTICE!  $* does not work at this time' ;fi

%.DO: %.CO
	@which -s co2ba || echo "Please install https://github.com/bkw777/dl2 for co2ba to generate $@"
	co2ba $< savem >$@

.PHONY: verify_%
verify_%: $(NAME)%.CO
	@cmp -l $< ../orig/$< && echo "Built $< is identical to reference ../orig/$<."

.PHONY: load_%
load_%: $(NAME)%.DO
	@which -s dl || echo "Please install https://github.com/bkw777/dl2 to bootstrap $<"
	dl -v -b $<

.PHONY: clean
clean:
	rm -f *.o *.bin *.sym *.lis *.map *.def *.CO *.DO
