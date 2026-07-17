libdir.x86_64  := $(shell if [ -d "/usr/lib/x86_64-linux-gnu" ]; then echo "/usr/lib/x86_64-linux-gnu"; else echo "/usr/lib64"; fi )
libdir.i686    := $(shell if [ -d "/usr/lib/i386-linux-gnu" ]; then echo "/usr/lib/i386-linux-gnu"; else echo "/usr/lib"; fi )
libdir.arm     := $(shell if [ -d "/usr/lib/arm-linux-gnueabihf" ]; then echo "/usr/lib/arm-linux-gnueabihf"; else echo "/usr/lib"; fi )
libdir.aarch64 := $(shell if [ -d "/usr/lib/aarch64-linux-gnu" ]; then echo "/usr/lib/aarch64-linux-gnu"; else echo "/usr/lib"; fi )
libdir.macos   := /usr/local/lib

ISNOTMACOS := $(shell uname -a | grep "Darwin" >/dev/null ; echo $$? )

ifeq ($(OS),Windows_NT)
	# Native Windows build under MSYS2 (MinGW gcc from a MINGW32/64 shell).
	MACHINE := windows
	PLUGIN := genericsqlformatter.dll
	CFLAGS := -shared -Wl,--export-all-symbols
	# MSYS2's Geany loads plugins from <libdir>/geany (e.g. /mingw64/lib/geany).
	libdir.windows := $(shell pkg-config --variable=libdir geany)
	# Per-user plugin dir of Geany on Windows.
	PLUGIN_USER_DIR := $(APPDATA)/geany/plugins
else ifeq ($(ISNOTMACOS), 0)
	MACHINE := macos
	PLUGIN := genericsqlformatter.so
	CFLAGS := -bundle
	PLUGIN_USER_DIR := $(HOME)/.config/geany/plugins
else
	MACHINE := $(shell uname -m)
	ifneq (, $(findstring armv, $(MACHINE)))
		 MACHINE := arm
	endif

	PLUGIN := genericsqlformatter.so
	CFLAGS := -shared
	PLUGIN_USER_DIR := $(HOME)/.config/geany/plugins
endif

libdir = $(libdir.$(MACHINE))/geany

all: prepare build

prepare:
	rm -fr ./fsqlf-1.0.0-prealpha-02/build && mkdir ./fsqlf-1.0.0-prealpha-02/build
	cd ./fsqlf-1.0.0-prealpha-02/build && CFLAGS="-fPIC" cmake ../lib_fsqlf
	cp -fr ./fsqlf-1.0.0-prealpha-02/include/* ./fsqlf-1.0.0-prealpha-02/build
	cd ./fsqlf-1.0.0-prealpha-02/build && cmake --build . --target libfsqlf_static

build:
	gcc -DLOCALEDIR=\"\" -DGETTEXT_PACKAGE=\"zhgzhg\" -DSTATIC_LIBFSQLF -c ./geany_generic_sql_formatter.c -fPIC `pkg-config --cflags geany`
	gcc geany_generic_sql_formatter.o -o $(PLUGIN) "././fsqlf-1.0.0-prealpha-02/build/liblibfsqlf_static.a" $(CFLAGS) `pkg-config --libs geany`

install: globaluninstall globalinstall localuninstall

uninstall: globaluninstall

globaluninstall:
	rm -f "$(libdir)/$(PLUGIN)"
	rm -f $(libdir)/genericsqlformatter.*

globalinstall:
	cp -f ./$(PLUGIN) "$(libdir)/$(PLUGIN)"
	chmod 755 "$(libdir)/$(PLUGIN)"

localinstall: localuninstall
	mkdir -p "$(PLUGIN_USER_DIR)"
	cp -f ./$(PLUGIN) "$(PLUGIN_USER_DIR)/$(PLUGIN)"
	chmod 755 "$(PLUGIN_USER_DIR)/$(PLUGIN)"

localuninstall:
	rm -f "$(PLUGIN_USER_DIR)/$(PLUGIN)"

clean:
	rm -f ./genericsqlformatter.so ./genericsqlformatter.dll
	rm -f ./geany_generic_sql_formatter.o
	rm -fr ./fsqlf-1.0.0-prealpha-02/build

.PHONY: all prepare build install uninstall globaluninstall globalinstall localinstall localuninstall clean
