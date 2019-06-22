# Name  : Junhong Wang
# ID    : 504941113
# Email : oneone@g.ucla.edu

default: lab4c_tcp lab4c_tls

lab4c_tcp: lab4c_tcp.c
	gcc -lmraa -lm -Wall -Wextra -o lab4c_tcp lab4c_tcp.c

lab4c_tls: lab4c_tls.c
	gcc -lmraa -lm -lssl -lcrypto -Wall -Wextra -o lab4c_tls lab4c_tls.c

clean:
	rm -f lab4c_tcp lab4c_tls lab4c-504941113.tar.gz

dist: lab4c_tcp.c lab4c_tls.c Makefile README
	tar -czvf lab4c-504941113.tar.gz lab4c_tcp.c lab4c_tls.c Makefile README
