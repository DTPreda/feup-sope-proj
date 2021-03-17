
CFLAGS = -Wall

xmod: xmod.o log.o sig_handling.o inoutput.o perms.o
	gcc -o xmod xmod.o log.o sig_handling.o inoutput.o perms.o


perms.o: perms.c perms.h 
	gcc $(CFLAGS) -c perms.c

inoutput.o: inoutput.c inoutput.h
	gcc $(CFLAGS) -c inoutput.c

sig_handling.o: sig_handling.c sig_handling.h
	gcc $(CFLAGS) -c sig_handling.c

log.o: log.c log.h
	gcc $(CFLAGS) -c log.c

xmod.o: xmod.c log.h
	gcc $(CFLAGS) -c xmod.c log.c

clean:
	rm *o xmod
