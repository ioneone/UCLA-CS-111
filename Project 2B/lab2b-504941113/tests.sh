# /bin/bash

# Name  : Junhong Wang
# ID    : 504941113
# Email : oneone@g.ucla.edu

rm -f lab2b_list.csv

# Mutex synchronized list operations, 1,000 iterations, 1,2,4,8,12,16,24 threads
# Spin-lock synchronized list operations, 1,000 iterations, 1,2,4,8,12,16,24 threads
for t in 1 2 4 8 12 16 24
do
	for s in m s
	do
		echo "./lab2_list --threads=$t --iterations=1000 --sync=$s >> lab2b_list.csv"
		./lab2_list --threads=$t --iterations=1000 --sync=$s >> lab2b_list.csv
	done
done

# Run your program with --yield=id, 4 lists, 1,4,8,12,16 threads, and 1, 2, 4, 8, 16 iterations 
# (and no synchronization) to see how many iterations it takes to reliably fail 
# (and make sure your Makefile expects some of these tests to fail).
for t in 1 4 8 12 16
do
	for i in 1 2 4 8 16
	do
		echo "./lab2_list --threads=$t --iterations=$i --yield=id --lists=4 >> lab2b_list.csv"
		./lab2_list --threads=$t --iterations=$i --yield=id --lists=4 >> lab2b_list.csv
	done
done

# Run your program with --yield=id, 4 lists, 1,4,8,12,16 threads, 
# and 10, 20, 40, 80 iterations, --sync=s and --sync=m to confirm 
# that updates are now properly protected (i.e., all runs succeeded).
for t in 1 4 8 12 16
do
	for i in 10 20 40 80
	do 
		for s in s m
		do
			echo "./lab2_list --threads=$t --iterations=$i --yield=id --sync=$s --lists=4 >> lab2b_list.csv"
			./lab2_list --threads=$t --iterations=$i --yield=id --sync=$s --lists=4 >> lab2b_list.csv
		done
	done
done

# Rerun both synchronized versions, without yields, 
# for 1000 iterations, 1,2,4,8,12 threads, and 1,4,8,16 lists.
for t in 1 2 4 8 12
do
	for s in s m
	do
		for l in 4 8 16
		do
			echo "./lab2_list --threads=$t --iterations=1000 --sync=$s --lists=$l >> lab2b_list.csv"
			./lab2_list --threads=$t --iterations=1000 --sync=$s --lists=$l >> lab2b_list.csv
		done
	done
done








