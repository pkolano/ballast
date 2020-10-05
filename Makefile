# install prefix for non-/etc items
PREFIX=/usr/local

all: c/ballast

c/ballast: c/ballast.c
	gcc -std=gnu99 $(CFLAGS) -o c/ballast c/ballast.c

clean:
	rm -f c/ballast

install: install_agent install_client install_server

install_common:
	install -d -m 755 $(PREFIX)/bin/ $(PREFIX)/sbin/
	install -d -m 755 $(PREFIX)/man/man1/ $(PREFIX)/man/man5/
	install -m 644 etc/ballastrc /etc/
	install -m 644 doc/ballastrc.5 $(PREFIX)/man/man5/

install_agent: install_common
	install -m 755 perl/ballast-agent $(PREFIX)/sbin/
	install -m 644 doc/ballast-agent.1 $(PREFIX)/man/man1/
	install -m 644 etc/systemd/system/ballast-agent.service /etc/systemd/system/
	install -m 644 etc/systemd/system/ballast-agent.timer /etc/systemd/system/

install_client: install_common
	install -m 755 perl/ballast $(PREFIX)/bin/
	install -m 644 doc/ballast.1 $(PREFIX)/man/man1/

install_c_client: c/ballast install_common
	install -m 755 c/ballast $(PREFIX)/bin/
	install -m 644 doc/ballast.1 $(PREFIX)/man/man1/

install_server: install_common
	install -m 755 perl/ballastd $(PREFIX)/sbin/
	install -m 644 doc/ballastd.1 $(PREFIX)/man/man1/
	install -m 644 etc/systemd/system/ballast.service /etc/systemd/system/
	install -m 644 etc/systemd/system/ballast.socket /etc/systemd/system/
