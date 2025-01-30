# Makefile for DeskLink2

OS ?= $(shell uname)
CC ?= gcc
CFLAGS += -O2 -Wall
#CFLAGS += -std=c99 -D_DEFAULT_SOURCE    # prove the code is still plain c
PREFIX ?= /usr/local
NAME := dl
APP_NAME := DeskLink2
APP_LIB_DIR := $(PREFIX)/lib/$(NAME)
APP_DOC_DIR := $(PREFIX)/share/doc/$(NAME)
APP_VERSION := $(shell git describe --long 2>&-)

# optional configurables
#FB100_ROM := Brother_FB-100.rom # exists but not used
#TPDD2_ROM := TANDY_26-3814.rom  # exists and is used
#DEFAULT_BASIC_BYTE_MS := 8  # ms per byte in bootstrap
#DEFAULT_MODEL := 1          # 1=tpdd1  2=tpdd2  (TS-DOS directory support requires tpdd1)
#DEFAULT_OPERATION_MODE := 1 # 0=FDC-mode 1=Operation-mode
#DEFAULT_BAUD := 19200
#DEFAULT_RTSCTS := false
#DEFAULT_UPCASE := false
#DEFAULT_PROFILE := "k85" # k85 = Floppy/TS-DOS/etc - 6.2, padded, F, dme, magic files
#RAW_ATTR := 0x20       # attr for "raw" mode, drive firmware fills unused fields with 0x20
#DEFAULT_TILDES := true
#XATTR_NAME := pdd.attr
#TSDOS_ROOT_LABEL := "0:    "
#TSDOS_PARENT_LABEL := "^     "

CLIENT_LOADERS := \
	clients/teeny/TINY.100 \
	clients/teeny/D.100 \
	clients/teeny/TEENY.100 \
	clients/teeny/TEENY.200 \
	clients/teeny/TEENY.NEC \
	clients/teeny/TEENY.M10 \
	clients/dskmgr/DSKMGR.100 \
	clients/dskmgr/DSKMGR.200 \
	clients/dskmgr/DSKMGR.K85 \
	clients/dskmgr/DSKMGR.M10 \
	clients/ts-dos/TSLOAD.100 \
	clients/ts-dos/TSLOAD.200 \
	clients/ts-dos/TS-DOS.100 \
	clients/ts-dos/TS-DOS.200 \
	clients/ts-dos/TS-DOS.NEC \
	clients/pakdos/PAKDOS.100 \
	clients/pakdos/PAKDOS.200 \
	clients/disk_power/Disk_Power.K85 \
#	clients/power-dos/POWR-D.100

LIB_OTHER := \
	$(TPDD2_ROM) \
	clients/ts-dos/DOS100.CO \
	clients/ts-dos/DOS200.CO \
	clients/ts-dos/DOSNEC.CO \
	clients/ts-dos/SAR100.CO \
	clients/ts-dos/SAR200.CO \
	clients/ts-dos/Sardine_American_English.pdd1 \
	clients/disk_power/Disk_Power.K85.pdd1 \

CLIENT_DOCS := \
	clients/teeny/teenydoc.txt \
	clients/teeny/hownec.do \
	clients/teeny/TNYO10.TXT \
	clients/teeny/tindoc.do \
	clients/teeny/ddoc.do \
	clients/dskmgr/DSKMGR.DOC \
	clients/ts-dos/tsdos.pdf \
	clients/pakdos/PAKDOS.DOC \
	clients/disk_power/Disk_Power.txt \
#	clients/power-dos/powr-d.txt

DOCS := dl.do README.txt README.md LICENSE $(CLIENT_DOCS)
SOURCES := main.c dir_list.c xattr.c
HEADERS := constants.h dir_list.h xattr.h

ifeq ($(OS),Darwin)
 TTY_PREFIX := cu.usbserial
else
 ifneq (,$(findstring BSD,$(OS)))
  TTY_PREFIX := ttyU
 else ifeq ($(OS),Linux)
  TTY_PREFIX := ttyUSB
 else
  TTY_PREFIX := ttyS
 endif
 LDLIBS += -lutil
endif

INSTALLOWNER = -o root
ifeq ($(OS),Windows_NT)
 INSTALLOWNER =
 CFLAGS += -D_WIN
endif

DEFS = \
	-DAPP_NAME=\"$(APP_NAME)\" \
	-DAPP_VERSION=\"$(APP_VERSION)\" \
	-DAPP_LIB_DIR=\"$(APP_LIB_DIR)\" \
	-DTTY_PREFIX=\"$(TTY_PREFIX)\" \
	-DUSE_XATTR \
#	-DPRINT_8BIT \
#	-DNADSBOX_EXTENSIONS \

#ifdef TPDD1_ROM
#	DEFS += -DTPDD1_ROM=\"$(TPDD1_ROM)\"
#endif
ifdef TPDD2_ROM
	DEFS += -DTPDD2_ROM=\"$(TPDD2_ROM)\"
endif
ifdef TSDOS_ROOT_LABEL
	DEFS += -DTSDOS_ROOT_LABEL=\"$(TSDOS_ROOT_LABEL)\"
endif
ifdef TSDOS_PARENT_LABEL
	DEFS += -DTSDOS_PARENT_LABEL=\"$(TSDOS_PARENT_LABEL)\"
endif
ifdef DEFAULT_BASIC_BYTE_MS
	DEFS += -DDEFAULT_BASIC_BYTE_MS=$(DEFAULT_BASIC_BYTE_MS)
endif
ifdef DEFAULT_MODEL
	DEFS += -DDEFAULT_MODEL=$(DEFAULT_MODEL)
endif
ifdef DEFAULT_OPERATION_MODE
	DEFS += -DDEFAULT_OPERATION_MODE=$(DEFAULT_OPERATION_MODE)
endif
ifdef DEFAULT_BAUD
	DEFS += -DDEFAULT_BAUD=$(DEFAULT_BAUD)
endif
ifdef DEFAULT_RTSCTS
	DEFS += -DDEFAULT_RTSCTS=$(DEFAULT_RTSCTS)
endif
ifdef DEFAULT_PROFILE
	DEFS += -DDEFAULT_PROFILE=$(DEFAULT_PROFILE)
endif
ifdef DEFAULT_UPCASE
	DEFS += -DDEFAULT_UPCASE=$(DEFAULT_UPCASE)
endif
ifdef DEFAULT_ATTR
	DEFS += -DDEFAULT_ATTR=$(DEFAULT_ATTR)
endif
ifdef RAW_ATTR
	DEFS += -DRAW_ATTR=$(RAW_ATTR)
endif
ifdef DEFAULT_TILDES
	DEFS += -DDEFAULT_TILDES=$(DEFAULT_TILDES)
endif
ifdef XATTR_NAME
	DEFS += -DXATTR_NAME=\"$(XATTR_NAME)\"
endif

DEFINES := $(DEFS)

ifdef DEBUG
 CFLAGS += -g
endif

.PHONY: all
all: $(NAME)

$(NAME): Makefile $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(DEFINES) $(SOURCES) $(LDLIBS) -o $(@)

install: $(NAME) $(CLIENT_LOADERS) $(LIB_OTHER) $(DOCS)
	mkdir -p $(APP_LIB_DIR)
	for s in $(CLIENT_LOADERS) ;do \
		d=$(APP_LIB_DIR)/$${s##*/} ; \
		install $(INSTALLOWNER) -m 0644 $${s} $${d} ; \
		[ ! -f $${s}.pre-install.txt ] && continue ;install $(INSTALLOWNER) -m 0644 $${s}.pre-install.txt $${d}.pre-install.txt ; \
		[ ! -f $${s}.post-install.txt ] && continue ;install $(INSTALLOWNER) -m 0644 $${s}.post-install.txt $${d}.post-install.txt ; \
	done
	for s in $(LIB_OTHER) ;do \
		d=$(APP_LIB_DIR)/$${s##*/} ; \
		install $(INSTALLOWNER) -m 0644 $${s} $${d} ; \
	done
	for s in $(DOCS) ;do \
		d=$(APP_DOC_DIR)/$${s} ; \
		mkdir -p $${d%/*} ; \
		install $(INSTALLOWNER) -m 0644 $${s} $${d} ; \
	done
	mkdir -p $(PREFIX)/bin
	install $(INSTALLOWNER) -m 0755 $(NAME) $(PREFIX)/bin/$(NAME)
	install $(INSTALLOWNER) -m 0755 co2ba.sh $(PREFIX)/bin/co2ba

uninstall:
	rm -rf $(APP_LIB_DIR) $(APP_DOC_DIR) $(PREFIX)/bin/$(NAME) $(PREFIX)/bin/co2ba

clean:
	rm -f $(NAME)
