# Name  : Junhong Wang
# ID    : 504941113
# Email : oneone@g.ucla.edu

default:
	rm -f lab3b
	ln -s lab3b.py lab3b
	chmod u+x lab3b
	echo "Compilation Succsessful"

dist: lab3b.py Makefile README trivial.csv
	tar -czvf lab3b-504941113.tar.gz lab3b.py Makefile README trivial.csv

test: trivial.csv
	./lab3b trivial.csv

clean:
	rm -f lab3b lab3b-504941113.tar.gz

