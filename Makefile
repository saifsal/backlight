# backlight - backlight utility
# See LICENSE file for copyright and license details.

SRC = backlight.c
OBJ = ${SRC:.c=.o}

all: backlight

.c.o:
	${CC} -c ${CFLAGS} $<

backlight: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm -f backlight ${OBJ}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f backlight ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/backlight

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/backlight

.PHONY: all clean install uninstall
