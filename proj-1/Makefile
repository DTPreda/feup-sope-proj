
CFLAGS = -Wall

xmod: xmod.o log.o
	gcc -o xmod xmod.o log.o

logs.o: logs.c logs.h
	gcc $(CFLAGS) -c logs.c

xmod.o: xmod.c log.h
	gcc $(CFLAGS) -c xmod.c

clean:
	rm *o xmod
