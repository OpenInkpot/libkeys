lib_LTLIBRARIES = libkeys.la
libkeys_la_SOURCES = libkeys.c
include_HEADERS = libkeys.h

AM_CFLAGS = -Wall --std=c99 -DSYSCONFDIR="\"$(sysconfdir)\"" -DDATADIR="\"$(datadir)\""

INCLUDES = $(EFL_CFLAGS)
libkeys_la_LDFLAGS = -Wl,--no-undefined $(EFL_LIBS)
