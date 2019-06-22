# Name  : Junhong Wang                                                         
# ID    : 504941113                                                            
# Email : oneone@g.ucla.edu

default: lab1a.c
	gcc -Wall -Wextra -g -o lab1a lab1a.c

clean:
	rm -f lab1a lab1a-504941113.tar.gz

dist: lab1a.c Makefile
	tar -czvf lab1a-504941113.tar.gz lab1a.c Makefile README
