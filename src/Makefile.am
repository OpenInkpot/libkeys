lib_LTLIBRARIES = libkeys.la
libkeys_la_SOURCES = libkeys.c
include_HEADERS = libkeys.h

AM_CFLAGS = -Wall -DSYSCONFDIR="\"$(sysconfdir)\"" -DDATADIR="\"$(datadir)\""

INCLUDES = $(EFL_CFLAGS)
libkeys_la_LDFLAGS = -Wl,--no-undefined $(EFL_LIBS)

noinst_PROGRAMS = keys_test 
keys_test_SOURCES = keys_test.c
keys_test_LDADD = libkeys.la $(EFL_LIBS)
