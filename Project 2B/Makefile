# Name  : Junhong Wang
# ID    : 504941113
# Email : oneone@g.ucla.edu

default: lab2_list

lab2_list: lab2_list.c SortedList.h SortedList.c
	gcc -Wall -Wextra -pthread -g -o $@ lab2_list.c SortedList.c

tests: lab2_list tests.sh
	chmod u+x tests.sh
	./tests.sh

profile: lab2_list
	rm -f profile.out lab2_list.prof
	LD_PRELOAD=/usr/local/cs/gperftools-2.7/lib/libprofiler.so.0 CPUPROFILE=lab2_list.prof ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text                               lab2_list lab2_list.prof >> profile.out
	pprof --list=thread_function               lab2_list lab2_list.prof >> profile.out
	pprof --list=thread_list_insert            lab2_list lab2_list.prof >> profile.out
	pprof --list=thread_list_length            lab2_list lab2_list.prof >> profile.out
	pprof --list=thread_list_delete            lab2_list lab2_list.prof >> profile.out
	pprof --list=spin_list_insert              lab2_list lab2_list.prof >> profile.out
	pprof --list=spin_list_length              lab2_list lab2_list.prof >> profile.out
	pprof --list=spin_list_delete              lab2_list lab2_list.prof >> profile.out
	pprof --list=pthread_mutex_lock_with_timer lab2_list lab2_list.prof >> profile.out
	pprof --list=__sync_lock_spin_with_timer   lab2_list lab2_list.prof >> profile.out

graphs: lab2_list.gp tests
	chmod u+x lab2_list.gp
	./lab2_list.gp

dist: Makefile README graphs
	tar -czvf lab2b-504941113.tar.gz lab2_list.c SortedList.h SortedList.c lab2_list.gp Makefile README tests.sh lab2b_list.csv profile.out lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png

clean:
	rm -f lab2_list lab2b-504941113.tar.gz lab2_list.prof