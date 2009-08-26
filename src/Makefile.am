lib_LTLIBRARIES = libkeys.la
libkeys_la_SOURCES = libkeys.c
include_HEADERS = libkeys.h

AM_CFLAGS = -Wall -DSYSCONFDIR="\"$(sysconfdir)\""

INCLUDES = $(EFL_CFLAGS)
libkeys_la_LDFLAGS = -Wl,--no-undefined $(EFL_LIBS)
