
CFLAGS = -Wall

xmod: xmod.o log.o
	gcc -o xmod xmod.o log.o

log.o: log.c log.h
	gcc $(CFLAGS) -c log.c

xmod.o: xmod.c log.h
	gcc $(CFLAGS) -c xmod.c log.c

clean:
	rm *o xmod
