# Name  : Junhong Wang
# ID    : 504941113
# Email : oneone@g.ucla.edu

default: lab4b

lab4b: lab4b.c
	gcc -Wall -Wextra -lm -lmraa -o lab4b lab4b.c

check: lab4b ./smokeTest.sh
	chmod u+x smokeTest.sh
	./smokeTest.sh

clean:
	rm -f lab4b lab4b-504941113.tar.gz

dist: lab4b.c smokeTest.sh Makefile README
	tar -czvf lab4b-504941113.tar.gz lab4b.c smokeTest.sh Makefile README
