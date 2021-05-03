all: s c #server

#s: server.o lib.o delay.c headers/delay.h
#	gcc -Wall -DDELAY=100 -o s delay.c lib.o server.o -pthread

c: client.c headers/common/common.h
	gcc -Wall -o c client.c -pthread

# server.o: server.c common.h
# 	gcc -Wall -c -o server.o server.c

# lib.o: lib.c lib.h
# 	gcc -Wall -c -o lib.o lib.c

s: server.c server.h lib.c lib.h headers/common/common.h queue.c queue.h
	gcc -Wall -pthread -lpthread -lrt queue.c server.c lib.c -o s

clean:
	rm -f s c server
