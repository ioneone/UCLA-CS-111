# Name  : Junhong Wang                                                         
# ID    : 504941113                                                            
# Email : oneone@g.ucla.edu

default: lab0.c
	gcc -Wall -Wextra -g -o lab0 lab0.c

check: lab0 check.sh
	chmod u+x check.sh
	./check.sh
	
clean:
	rm -f lab0 lab0-504941113.tar.gz

dist: lab0.c Makefile check.sh backtrace.png breakpoint.png README
	tar -czvf lab0-504941113.tar.gz lab0.c Makefile check.sh backtrace.png breakpoint.png README



