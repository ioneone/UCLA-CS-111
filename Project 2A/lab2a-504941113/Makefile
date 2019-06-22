# Name  : Junhong Wang
# ID    : 504941113
# Email : oneone@g.ucla.edu

default: build

build: lab2_add lab2_list

lab2_add: lab2_add.c
	gcc -Wall -Wextra -lpthread -o $@ lab2_add.c

lab2_list: lab2_list.c SortedList.h SortedList.c
	gcc -Wall -Wextra -lpthread -o $@ lab2_list.c SortedList.c

tests: build tests.sh
	chmod u+x tests.sh
	./tests.sh

graphs: lab2_add.gp lab2_list.gp tests
	chmod u+x lab2_add.gp
	chmod u+x lab2_list.gp
	./lab2_add.gp
	./lab2_list.gp

dist: Makefile README graphs
	tar -czvf lab2a-504941113.tar.gz lab2_add.c lab2_list.c SortedList.h SortedList.c lab2_add.gp lab2_list.gp Makefile README tests.sh lab2_add.csv lab2_list.csv lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png

clean:
	rm -f lab2_add lab2_list lab2a-504941113.tar.gz
