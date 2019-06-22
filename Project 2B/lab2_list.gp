#! /usr/local/cs/bin/gnuplot
#
# Name  : Junhong Wang
# ID    : 504941113
# Email : oneone@g.ucla.edu
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#
# output:
#	lab2b_1.png ... throughput vs. number of threads for mutex and spin-lock synchronized list operations
#	lab2b_2.png ... mean time per mutex wait and mean time per operation for mutex-synchronized list operations
#	lab2b_3.png ... successful iterations vs. threads for each synchronization method
#	lab2b_4.png ... throughput vs. number of threads for mutex synchronized partitioned lists
#   lab2b_5.png ... throughput vs. number of threads for spin-lock-synchronized partitioned lists
#
# Note:
#	Managing data is simplified by keeping all of the results in a single
#	file.  But this means that the individual graphing commands have to
#	grep to select only the data they want.
#
#	Early in your implementation, you will not have data for all of the
#	tests, and the later sections may generate errors for missing data.
#

# general plot parameters
set terminal png
set datafile separator ","
billion = 1000000000

# throughput vs. number of threads for mutex and spin-lock synchronized list operations
set title "List-1: throughput vs. number of threads for mutex and spin-lock"
set xlabel "number of threads"
set logscale x 2
set xrange [0.75:]
set ylabel "throughput (operations/second)"
set logscale y 10
set output 'lab2b_1.png'

plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(billion/($7)) \
	title 'mutex' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(billion/($7)) \
	title 'spin-lock' with linespoints lc rgb 'green'

# mean time per mutex wait and mean time per operation for mutex-synchronized list operations
set title "List-2: mean time per mutex wait and mean time per operation"
set xlabel "number of threads"
set logscale x 2
set xrange [0.75:]
set ylabel "mean time"
set logscale y 10
set output 'lab2b_2.png'

plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8) \
	title 'per mutext wait' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) \
	title 'per operation w/ mutex' with linespoints lc rgb 'green'

# successful iterations vs. threads for each synchronization method
set title "List-3: successful iterations vs. threads"
set xlabel "number of threads"
set logscale x 2
set xrange [0.75:]
set ylabel "successful iterations"
set logscale y 10
set output 'lab2b_3.png'

plot \
     "< grep 'list-id-none,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'no sync' with points lc rgb 'red', \
	 "< grep 'list-id-m,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'mutex' with points lc rgb 'green', \
     "< grep 'list-id-s,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'spin' with points lc rgb 'blue'

# throughput vs. number of threads for mutex synchronized partitioned lists
set title "List-4: throughput vs. number of threads"
set xlabel "number of threads"
set logscale x 2
set xrange [0.75:]
set ylabel "throughput"
set logscale y 10
set output 'lab2b_4.png'

plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(billion/($7)) \
	title 'lists=1' with linespoints lc rgb 'red', \
	 "< grep 'list-none-m,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(billion/($7)) \
	title 'lists=4' with linespoints lc rgb 'green', \
     "< grep 'list-none-m,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(billion/($7)) \
	title 'lists=8' with linespoints lc rgb 'blue', \
	 "< grep 'list-none-m,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(billion/($7)) \
	title 'lists=16' with linespoints lc rgb 'black'

# throughput vs. number of threads for spin-lock-synchronized partitioned lists
set title "List-5: throughput vs. number of threads"
set xlabel "number of threads"
set logscale x 2
set xrange [0.75:]
set ylabel "throughput"
set logscale y 10
set output 'lab2b_5.png'

plot \
     "< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(billion/($7)) \
	title 'lists=1' with linespoints lc rgb 'red', \
	 "< grep 'list-none-s,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(billion/($7)) \
	title 'lists=4' with linespoints lc rgb 'green', \
     "< grep 'list-none-s,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(billion/($7)) \
	title 'lists=8' with linespoints lc rgb 'blue', \
	 "< grep 'list-none-s,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(billion/($7)) \
	title 'lists=16' with linespoints lc rgb 'black'









