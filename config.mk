# sic version
VERSION = 1.3

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

# includes and libs
INCS = -I. -I/usr/include
LIBS = -L/usr/lib -lc

# flags
CPPFLAGS = -DVERSION=\"${VERSION}\"
CFLAGS = -pedantic -Wall -Os ${INCS} ${CPPFLAGS}
LDFLAGS = -s ${LIBS}

# compiler and linker
CC = cc
