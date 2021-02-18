# bic - simple irc client

include config.mk

SRC = bic.c
OBJ = ${SRC:.c=.o}

all: options bic

options:
	@echo bic build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.h config.mk strlcpy.c util.c

config.h:
	@echo creating $@ from config.def.h
	@cp config.def.h $@

bic: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f bic ${OBJ} bic-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p bic-${VERSION}
	@cp -R LICENSE Makefile README arg.h config.def.h config.mk bic.1 bic.c util.c strlcpy.c bic-${VERSION}
	@tar -cf bic-${VERSION}.tar bic-${VERSION}
	@gzip bic-${VERSION}.tar
	@rm -rf bic-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f bic ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/bic
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@sed "s/VERSION/${VERSION}/g" < bic.1 > ${DESTDIR}${MANPREFIX}/man1/bic.1
	@chmod 644 ${DESTDIR}${MANPREFIX}/man1/bic.1

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/bic
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/bic.1

.PHONY: all options clean dist install uninstall
