all: s client

s: server.o lib.o delay.c delay.h
	gcc -Wall -DDELAY=0 -o s delay.c lib.o server.o -lpthread

client: client.c
	gcc -Wall client.c -o client -lpthread

# server.o: server.c
# 	gcc -Wall -c -o server.o server.c

# lib.o: lib.c lib.h
# 	gcc -Wall -c -o lib.o lib.c

clean:
	rm -f s c
