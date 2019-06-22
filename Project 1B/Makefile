# Name  : Junhong Wang                                                         
# ID    : 504941113                                                            
# Email : oneone@g.ucla.edu

default: client server

client: lab1b-client.c
	gcc -Wall -Wextra -lz -o lab1b-client lab1b-client.c

server: lab1b-server.c
	gcc -Wall -Wextra -lz -o lab1b-server lab1b-server.c

clean:
	rm -f lab1b-client lab1b-server lab1b-504941113.tar.gz

dist: lab1b-client.c lab1b-server.c Makefile README
	tar -czvf lab1b-504941113.tar.gz lab1b-client.c lab1b-server.c Makefile README