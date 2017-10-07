libdir.x86_64 := $(shell if [ -d "/usr/lib/x86_64-linux-gnu" ]; then echo "/usr/lib/x86_64-linux-gnu"; else echo "/usr/lib64"; fi )
libdir.i686   := $(shell if [ -d "/usr/lib/i386-linux-gnu" ]; then echo "/usr/lib/i386-linux-gnu"; else echo "/usr/lib"; fi )

MACHINE := $(shell uname -m)

libdir = $(libdir.$(MACHINE))

all: prepare build

prepare:
	rm -fr ./fsqlf-1.0.0-prealpha-02/build && mkdir ./fsqlf-1.0.0-prealpha-02/build
	cd ./fsqlf-1.0.0-prealpha-02/build && cmake ../lib_fsqlf
	cp -fr ./fsqlf-1.0.0-prealpha-02/include/* ./fsqlf-1.0.0-prealpha-02/build
	cd ./fsqlf-1.0.0-prealpha-02/build && make libfsqlf_static

build:
	gcc -DLOCALEDIR=\"\" -DGETTEXT_PACKAGE=\"zhgzhg\" -c ./geany_generic_sql_formatter.c -fPIC `pkg-config --cflags geany`
	gcc geany_generic_sql_formatter.o -o genericsqlformatter.so "././fsqlf-1.0.0-prealpha-02/build/liblibfsqlf_static.a" -shared `pkg-config --libs geany`

install: uninstall startinstall

startinstall:
	cp -f ./genericsqlformatter.so $(libdir)/geany
	chmod 755 $(libdir)/geany/genericsqlformatter.so

uninstall:
	rm -f $(libdir)/geany/genericsqlformatter.*

clean:
	rm -f ./genericsqlformatter.so
	rm -f ./genericsqlformatter.o
	rm -fr ./fsqlf-1.0.0-prealpha-02/build
