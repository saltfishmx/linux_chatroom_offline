all:client server
client:client.c
	gcc -c client.c
	gcc -o client client.o -lpthread
server:server.c
	gcc -c server.c
	gcc -o server server.o -lpthread
.PHONY:clean
clean:
	rm  *.o

