all: compile

compile:
	. ./makeenv.sh; $(MAKE) c/ballast

c/ballast: c/ballast.c
	gcc -std=gnu99 $(CFLAGS) -o c/ballast c/ballast.c

clean:
	rm -f c/ballast
