all: compile

VERSION = 1.5

compile:
	. ./makeenv.sh; $(MAKE) c/ballast

c/ballast: c/ballast.c
	gcc -std=gnu99 $(CFLAGS) -o c/ballast c/ballast.c

clean:
	rm -f c/ballast

dist:
	mkdir ../ballast-$(VERSION)
	cp -r * ../ballast-$(VERSION)
	rm -f ../ballast-$(VERSION)/.todo
	find -d ../ballast-$(VERSION) -name CVS -exec rm -rf {} \;
	tar zcvf ../ballast-$(VERSION).tgz ../ballast-$(VERSION)
	rm -rf ../ballast-$(VERSION)
