# Makefile for DeskLink+

OS := $(shell uname)
CFLAGS += -O2 -Wall
PREFIX = /usr/local
APP_NAME = dl
APP_LIB_DIR = $(PREFIX)/lib/$(APP_NAME)
APP_DOC_DIR = $(PREFIX)/share/doc/$(APP_NAME)

DEFAULT_CLIENT_TTY = ttyUSB0
DEFAULT_CLIENT_MODEL = 100
DEFAULT_CLIENT_APP = TEENY

TEENY_INSTALLERS = clients/teeny/TEENY.100 clients/teeny/TEENY.200 clients/teeny/TEENY.NEC
TEENY_DOCS = clients/teeny/teenydoc.txt
DSKMGR_INSTALLERS = clients/dskmgr/DSKMGR.100 clients/dskmgr/DSKMGR.200 clients/dskmgr/DSKMGR.K85 clients/dskmgr/DSKMGR.M10
DSKMGR_DOCS = clients/dskmgr/DSKMGR.DOC
#TS-DOS_INSTALLERS = clients/ts-dos/TS-DOS.100 clients/ts-dos/TS-DOS.200 clients/ts-dos/TS-DOS.NEC
#TS-DOS_DOCS = clients/ts-dos/ts-dos.txt
#POWR-D_INSTALLERS = clients/power-dos/POWR-D.100
#POWR-D_DOCS = clients/power-dos-dos/ts-dos.txt
#TINY_INSTALLERS = clients/tiny/TINY.100
#TINY_DOCS = clients/tiny/tiny.txt

CLIENT_APP_INSTALLERS = $(TEENY_INSTALLERS) $(DSKMGR_INSTALLERS)
CLIENT_APP_DOCS = $(TEENY_DOCS) $(DSKMGR_DOCS)
#CLIENT_APP_INSTALLERS = $(TEENY_INSTALLERS) $(TINY_INSTALLERS) $(TS-DOS_INSTALLERS) $(DSKMGR_INSTALLERS)
#CLIENT_APP_DOCS = $(TEENY_DOCS) $(TINY_DOCS) $(TS-DOS_DOCS) $(DSKMGR_DOCS)

DOCS = dl.do README.txt README.md LICENSE $(CLIENT_APP_DOCS)
SOURCES = dl.c dir_list.c

DEFINES = \
	-DAPP_LIB_DIR=$(APP_LIB_DIR) \
	-DDEFAULT_CLIENT_TTY=$(DEFAULT_CLIENT_TTY) \
	-DDEFAULT_CLIENT_APP=$(DEFAULT_CLIENT_APP) \
	-DDEFAULT_CLIENT_MODEL=$(DEFAULT_CLIENT_MODEL)

ifdef DEBUG
 CFLAGS += -g
else
 CFLAGS += -s
endif

ifeq ($(OS),Darwin)
else
 LDFLAGS += -lutil
endif

.PHONY: all
all: $(APP_NAME)

$(APP_NAME): $(SOURCES)
	gcc $(CFLAGS) $(DEFINES) $(SOURCES) $(LDFLAGS) -o $(@)

install: $(APP_NAME) $(CLIENT_APP_INSTALLERS) $(DOCS)
	install -o root -m 0755 -d $(APP_LIB_DIR) $(APP_DOC_DIR)
	install -o root -m 0644 -t $(APP_LIB_DIR) $(CLIENT_APP_INSTALLERS)
	for i in $(CLIENT_APP_INSTALLERS) ;do install -o root -m 0644 -t $(APP_LIB_DIR) $${i} $${i}.pre-install.txt $${i}.post-install.txt ;done
	install -o root -m 0644 -t $(APP_DOC_DIR) $(DOCS)
	install -o root -m 0755 $(APP_NAME) $(PREFIX)/bin/$(APP_NAME)

uninstall:
	rm -rf $(APP_LIB_DIR) $(APP_DOC_DIR) $(PREFIX)/bin/$(APP_NAME)

clean:
	rm -f $(APP_NAME)
