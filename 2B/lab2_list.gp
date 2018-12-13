# NAME: CHEN CHEN
# EMAIL: chenchenstudent@gmail.com
# ID: 004710308



# general plot parameters
set terminal png
set datafile separator ","


set title "NO1: # of Threads VS Throughput：operations/sec"
set xlabel "# of Threads"
set ylabel "Throughput (ops/sec)"
set xrange [1:25]
set logscale x 2
set logscale y 10
set output 'lab2b_1.png'

plot \
	"< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Mutex' with linespoints lc rgb 'blue', \
	"< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Spin-Lock' with linespoints lc rgb 'red'

##################################################

set title "NO2: # of threads vs time/operation"
set xlabel "# of Threads"
set ylabel "time/operation"
set xrange [1:25]
set logscale x 2
set logscale y 10
set output 'lab2b_2.png'

plot \
	"< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8) \
        title 'wait-for-lock-time' with linespoints lc rgb 'blue', \
        "< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) \
        title 'average time per operation' with linespoints lc rgb 'red'

##################################################

set title "NO3: # of threads VS Successful Iterations"
set xlabel "# of Threads"
set ylabel "Successful Iterations"
set xrange [0.75:]
set logscale x 2
set logscale y 10
set output 'lab2b_3.png'

plot \
	"< grep 'list-id-none' lab2b_list.csv | grep '^list'" using ($2):($3) \
        title 'unprotected w/yields' with points lc rgb 'red', \
	"< grep list-id-m lab2b_list.csv | grep '^list'" using ($2):($3) \
        title 'mutex w/yields' with points lc rgb 'blue', \
	"< grep list-id-s lab2b_list.csv | grep '^list'" using ($2):($3) \
        title 'spin w/yields' with points lc rgb 'orange'

##################################################

set title "NO4: # of threads VS Throughput with spin_lock operations"
set xlabel "# of Threads"
set ylabel "Throughput (ops/sec) with spin_lock"
set xrange [0.75:]
set logscale x 2
set logscale y 10
set output 'lab2b_4.png'

plot \
        "< grep 'list-none-m,.*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
        title '1 list' with linespoints lc rgb 'blue', \
	"< grep 'list-none-m,.*,1000,4' lab2b_list.csv" using ($2):(1000000000/($7)) \
        title '4 lists' with linespoints lc rgb 'green', \
	"< grep 'list-none-m,.*,1000,8' lab2b_list.csv" using ($2):(1000000000/($7)) \
        title '8 lists' with linespoints lc rgb 'red', \
	"< grep 'list-none-m,.*,1000,16' lab2b_list.csv" using ($2):(1000000000/($7)) \
        title '16 lists' with linespoints lc rgb 'orange'


set title "NO5: # of threads VS Throughput with mutex_lock operations"
set xlabel "# of Threads"
set ylabel "Throughput (ops/sec) with mutex_lock"
set xrange [0.75:]
set logscale x 2
set logscale y 10
set output 'lab2b_5.png'

plot \
        "< grep 'list-none-s,.*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
        title '1 list' with linespoints lc rgb 'blue', \
        "< grep 'list-none-s,.*,1000,4' lab2b_list.csv" using ($2):(1000000000/($7)) \
        title '4 lists' with linespoints lc rgb 'green', \
        "< grep 'list-none-s,.*,1000,8' lab2b_list.csv" using ($2):(1000000000/($7)) \
        title '8 lists' with linespoints lc rgb 'red', \
        "< grep 'list-none-s,.*,1000,16' lab2b_list.csv" using ($2):(1000000000/($7)) \
        title '16 lists' with linespoints lc rgb 'orange'
