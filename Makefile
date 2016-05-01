CFLAGS = -Wall -g -O3
LDFLAGS = -lSDL2 -lgsl -lgslcblas -lm

yoyo: yoyo.c main.c yoyo.h vec.h Makefile
	$(LINK.c) -o $@ yoyo.c main.c
clean:
	rm -fv yoyo
