# Name  : Junhong Wang
# ID    : 504941113
# Email : oneone@g.ucla.edu

default:
	gcc -Wall -Wextra -o lab3a lab3a.c

dist: lab3a.c ext2_fs.h Makefile README test.sh trivial.img trivial.csv
	tar -czvf lab3a-504941113.tar.gz lab3a.c ext2_fs.h Makefile README test.sh trivial.img trivial.csv

test: test.sh trivial.img trivial.csv
	chmod u+x test.sh
	./test.sh

clean:
	rm -f lab3a lab3a-504941113.tar.gz

