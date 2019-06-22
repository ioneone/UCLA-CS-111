# /bin/bash

# Name  : Junhong Wang
# ID    : 504941113
# Email : oneone@g.ucla.edu

###########################
# Part 1
###########################

rm -f lab2_add.csv

# Run your program for ranges of iterations (100, 1000, 10000, 100000) values
for t in 2 4 8 12
do
	for i in 100 1000 10000 100000
	do
		echo "./lab2_add --threads=$t --iterations=$i >> lab2_add.csv"
		./lab2_add --threads=$t --iterations=$i >> lab2_add.csv
	done
done

# Re-run your tests, with yields, for ranges of threads (2,4,8,12) 
# and iterations (10, 20, 40, 80, 100, 1000, 10000, 100000)
for t in 2 4 8 12
do
	for i in 10 20 40 80 100 1000 10000 100000
	do
		echo "./lab2_add --threads=$t --iterations=$i --yield >> lab2_add.csv"
		./lab2_add --threads=$t --iterations=$i --yield >> lab2_add.csv
	done
done

# Compare the average execution time of the yield and non-yield versions 
# a range threads (2, 8) and of iterations (100, 1000, 10000, 100000)
for t in 2 8
do
	for i in 100 1000 10000 100000
	do
		echo "./lab2_add --threads=$t --iterations=$i          >> lab2_add.csv"
		./lab2_add --threads=$t --iterations=$i          >> lab2_add.csv
		echo "./lab2_add --threads=$t --iterations=$i  --yield >> lab2_add.csv"
		./lab2_add --threads=$t --iterations=$i  --yield >> lab2_add.csv
	done
done

# Plot (using the supplied data reduction script), for a single thread, 
# the average cost per operation (non-yield) as a function of the number of iterations
for i in 10 20 40 80 100 1000 10000 100000 1000000
do
	echo "./lab2_add --threads=1 --iterations=$i >> lab2_add.csv"
	./lab2_add --threads=1 --iterations=$i >> lab2_add.csv
done

# Use your --yield options to confirm that, even for large numbers of 
# threads (2, 4, 8, 12) and iterations 
# (10,000 for mutexes and CAS, only 1,000 for spin locks)
for t in 2 4 8 12
do
	echo "./lab2_add --threads=$t --iterations=10000  --yield --sync=m >> lab2_add.csv"
	./lab2_add --threads=$t --iterations=10000  --yield --sync=m >> lab2_add.csv
	echo "./lab2_add --threads=$t --iterations=10000  --yield --sync=c >> lab2_add.csv"
	./lab2_add --threads=$t --iterations=10000  --yield --sync=c >> lab2_add.csv
	echo "./lab2_add --threads=$t --iterations=1000   --yield --sync=s >> lab2_add.csv"
	./lab2_add --threads=$t --iterations=1000   --yield --sync=s >> lab2_add.csv
done

# Using a large enough number of iterations (e.g. 10,000) to overcome the issues raised 
# in the question 2.1.3, test all four (no yield) versions 
# (unprotected, mutex, spin-lock, compare-and-swap) 
# for a range of number of threads (1,2,4,8,12)
for t in 1 2 4 8 12
do
	echo "./lab2_add --threads=$t --iterations=10000           >> lab2_add.csv"
	./lab2_add --threads=$t --iterations=10000           >> lab2_add.csv

	for s in m c s
	do
		echo "./lab2_add --threads=$t --iterations=10000  --sync=$s >> lab2_add.csv"
		./lab2_add --threads=$t --iterations=10000  --sync=$s >> lab2_add.csv
	done
done

###########################
# Part 2
###########################

rm -f lab2_list.csv

# Run your program with a single thread, and increasing 
# numbers of iterations (10, 100, 1000, 10000, 20000)
for i in 10 100 1000 10000 20000
do
	echo "./lab2_list --threads=1 --iterations=$i >> lab2_list.csv"
	./lab2_list --threads=1 --iterations=$i >> lab2_list.csv
done

# Run your program and see how many parallel threads (2,4,8,12) 
# and iterations (1, 10,100,1000) it takes to fairly consistently demonstrate a problem.
for t in 2 4 8 12
do
	for i in 1 10 100 1000
	do
		echo "./lab2_list --threads=$t --iterations=$i >> lab2_list.csv"
		./lab2_list --threads=$t --iterations=$i >> lab2_list.csv
	done
done

# Then run it again using various combinations of yield options and see 
# how many threads (2,4,8,12) and iterations (1, 2,4,8,16,32) it takes to 
# fairly consistently demonstrate the problem ()
for t in 2 4 8 12
do
	for i in 1 2 4 8 16 32
	do
		for y in i d l id il dl
		do
			echo "./lab2_list --threads=$t --iterations=$i --yield=$y  >> lab2_list.csv" 
			./lab2_list --threads=$t --iterations=$i --yield=$y  >> lab2_list.csv
		done
	done
done

# Using your --yield options, demonstrate that either of these protections seems to 
# eliminate all of the problems, even for moderate numbers of threads (12) and iterations (32)
for y in i d l id il dl
do
	for s in m s
	do
		echo "./lab2_list --threads=12 --iterations=32 --yield=$y   --sync=$s >> lab2_list.csv"
		./lab2_list --threads=12 --iterations=32 --yield=$y   --sync=$s >> lab2_list.csv
	done
done

# Choose an appropriate number of iterations (e.g. 1000) to overcome start-up costs 
# and rerun your program without the yields for a range of # threads (1, 2, 4, 8, 12, 16, 24).
for t in 1 2 4 8 12 16 24
do
	echo "./lab2_list --threads=$t --iterations=1000          >> lab2_list.csv"
	./lab2_list --threads=$t --iterations=1000          >> lab2_list.csv

	for s in m s
	do
		echo "./lab2_list --threads=$t --iterations=1000 --sync=$s >> lab2_list.csv"
		./lab2_list --threads=$t --iterations=1000 --sync=$s >> lab2_list.csv
	done

done









