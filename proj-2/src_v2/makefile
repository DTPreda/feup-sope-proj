all: s c #server

#s: server.o lib.o delay.c headers/delay.h
#	gcc -Wall -DDELAY=100 -o s delay.c lib.o server.o -pthread

c: client.c headers/queue/common/common.h
	gcc -Wall -o c client.c -pthread

# server.o: server.c common.h
# 	gcc -Wall -c -o server.o server.c


s: server.c headers/server.h lib.c headers/lib/lib.h headers/queue/common/common.h queue.c headers/queue/queue.h headers/delay.h delay.c
	gcc -Wall -DDELAY=100 -pthread -lpthread -lrt queue.c server.c lib.c delay.c -o s

clean:
	rm -f s c server
