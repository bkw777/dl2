# Makefile for legacy TEENY for 200 disassembly
# Requires https://github.com/z88dk/z88dk
# Requires https://github.com/bkw777/dl2

NAME = TNY200
SRC = $(NAME).S85

AS = z88dk-z80asm
ASFLAGS := -m8085_strict -no-synth # -v
# z88dk-z80asm < 25000 has a bug that it only accepts the zilog syntax for RST.
# If $(AS) is z88dk then add -DZ88DK=ver to flags. Used in m100_common.S85
# fixed as of https://github.com/z88dk/z88dk/commit/db5c165
ASFLAGS += $(shell $(AS) |awk '/^Z80 Macro Assembler /{split($$4,a,"-");if (a[1]>0) print "-DZ88DK="a[1]}')

# -DHIMEM=addr         # generate a relocated binary with specified HIMEM/END address
XFLAGS :=

# don't delete .CO when building to get to .DO
.SECONDARY:

.PHONY: all
all: verify $(NAME).DO

$(NAME).CO: $(SRC)
	$(AS) $(ASFLAGS) $(XFLAGS) -b -o=$@ $< && \
	$(AS) $(ASFLAGS) $(XFLAGS) -DPRGLEN=$$(wc -c <$@) -b -o=$@ $<
	@rm *.o

%.DO: %.CO
	@which -s co2ba || echo "Please install https://github.com/bkw777/dl2 for co2ba to generate $@"
	co2ba $< savem >$@

.PHONY: verify
verify: $(NAME).CO
	@cmp -l $< ../$< && echo "Built $< is identical to reference ../$<."

.PHONY: load
load: $(NAME).DO
	@which -s dl || echo "Please install https://github.com/bkw777/dl2 to bootstrap $<"
	dl -v -b $<

.PHONY: clean
clean:
	rm -f *.o *.bin *.sym *.lis *.map *.def *.CO *.DO
