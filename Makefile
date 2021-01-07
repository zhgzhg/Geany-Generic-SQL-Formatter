libdir.x86_64 := $(shell if [ -d "/usr/lib/x86_64-linux-gnu" ]; then echo "/usr/lib/x86_64-linux-gnu"; else echo "/usr/lib64"; fi )
libdir.i686   := $(shell if [ -d "/usr/lib/i386-linux-gnu" ]; then echo "/usr/lib/i386-linux-gnu"; else echo "/usr/lib"; fi )
libdir.arm    := $(shell if [ -d "/usr/lib/arm-linux-gnueabihf" ]; then echo "/usr/lib/arm-linux-gnueabihf"; else echo "/usr/lib"; fi )
libdir.macos  := /usr/local/lib

ISNOTMACOS := $(shell uname -a | grep "Darwin" >/dev/null ; echo $$? )

ifeq ($(ISNOTMACOS), 0)
	MACHINE := macos
	CFLAGS := -bundle
else
	MACHINE := $(shell uname -m)
	ifneq (, $(findstring armv, $(MACHINE)))
		 MACHINE := arm
	endif
	CFLAGS := -shared
endif

libdir = $(libdir.$(MACHINE))/geany

all: prepare build

prepare:
	rm -fr ./fsqlf-1.0.0-prealpha-02/build && mkdir ./fsqlf-1.0.0-prealpha-02/build
	cd ./fsqlf-1.0.0-prealpha-02/build && CFLAGS="-fPIC" cmake ../lib_fsqlf
	cp -fr ./fsqlf-1.0.0-prealpha-02/include/* ./fsqlf-1.0.0-prealpha-02/build
	cd ./fsqlf-1.0.0-prealpha-02/build && make libfsqlf_static

build:
	gcc -DLOCALEDIR=\"\" -DGETTEXT_PACKAGE=\"zhgzhg\" -c ./geany_generic_sql_formatter.c -fPIC `pkg-config --cflags geany`
	gcc geany_generic_sql_formatter.o -o genericsqlformatter.so "././fsqlf-1.0.0-prealpha-02/build/liblibfsqlf_static.a" $(CFLAGS) `pkg-config --libs geany`

install: globaluninstall globalinstall localuninstall

uninstall: globaluninstall

globaluninstall:
	rm -f "$(libdir)/genericsqlformatter.so"
	rm -f $(libdir)/genericsqlformatter.*

globalinstall:
	cp -f ./genericsqlformatter.so "$(libdir)/genericsqlformatter.so"
	chmod 755 "$(libdir)/genericsqlformatter.so"

localinstall: localuninstall
	mkdir -p "$(HOME)/.config/geany/plugins"
	cp -f ./genericsqlformatter.so "$(HOME)/.config/geany/plugins/genericsqlformatter.so"
	chmod 755 "$(HOME)/.config/geany/plugins/genericsqlformatter.so"

localuninstall:
	rm -f "$(HOME)/.config/geany/plugins/genericsqlformatter.so"

clean:
	rm -f ./genericsqlformatter.so
	rm -f ./genericsqlformatter.o
	rm -fr ./fsqlf-1.0.0-prealpha-02/build
