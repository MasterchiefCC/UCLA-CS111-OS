# NAME: CHEN CHEN
# EMAIL: chenchenstudent@gmail.com
# ID: 004710308
CC = gcc
CFLAGS = -g3 -O0 -Wall -Wextra -pthread -lrt

default: build

build:
	$(CC) lab2_list.c SortedList.c -o lab2_list $(CFLAGS)

clean:
	rm -f *.o *.tar.gz lab2_list

graphs:tests
	gnuplot lab2_list.gp

dist: clean build graphs profile
	tar -zcvf lab2b-004710308.tar.gz *.c *.h *.png *.csv *.out *.gp README Makefile 


profile:
	-rm -f ywwuyi.wuyikoei 
	LD_PRELOAD=/usr/lib64/libprofiler.so CPUPROFILE=ywwuyi.wuyikoei ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text lab2_list ywwuyi.wuyikoei > profile.out
	pprof --list=listWork lab2_list ywwuyi.wuyikoei >> profile.out
	-rm -f ywwuyi.wuyikoei

tests:
	echo " " > lab2b_list.csv
	./lab2_list --threads=1  --iterations=1000 --sync=m   >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000 --sync=m   >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000 --sync=m   >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000 --sync=m   >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --sync=m   >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=1000 --sync=m   >> lab2b_list.csv
	./lab2_list --threads=24 --iterations=1000 --sync=m   >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=1000 --sync=s   >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000 --sync=s   >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000 --sync=s   >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000 --sync=s   >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000 --sync=s   >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=1000 --sync=s   >> lab2b_list.csv
	./lab2_list --threads=24 --iterations=1000 --sync=s   >> lab2b_list.csv
	-./lab2_list --threads=1  --iterations=1   --yield=id --lists=4   >> lab2b_list.csv
	-./lab2_list --threads=1  --iterations=2   --yield=id --lists=4   >> lab2b_list.csv
	-./lab2_list --threads=1  --iterations=4   --yield=id --lists=4   >> lab2b_list.csv
	-./lab2_list --threads=1  --iterations=8   --yield=id --lists=4   >> lab2b_list.csv
	-./lab2_list --threads=1  --iterations=16  --yield=id --lists=4   >> lab2b_list.csv
	-./lab2_list --threads=4  --iterations=1   --yield=id --lists=4   >> lab2b_list.csv
	-./lab2_list --threads=4  --iterations=2   --yield=id --lists=4   >> lab2b_list.csv
	-./lab2_list --threads=4  --iterations=4   --yield=id --lists=4   >> lab2b_list.csv
	-./lab2_list --threads=4  --iterations=8   --yield=id --lists=4   >> lab2b_list.csv
	-./lab2_list --threads=4  --iterations=16  --yield=id --lists=4   >> lab2b_list.csv
	-./lab2_list --threads=8  --iterations=1   --yield=id --lists=4   >> lab2b_list.csv
	-./lab2_list --threads=8  --iterations=2   --yield=id --lists=4   >> lab2b_list.csv
	-./lab2_list --threads=8  --iterations=4   --yield=id --lists=4   >> lab2b_list.csv
	-./lab2_list --threads=8  --iterations=8   --yield=id --lists=4   >> lab2b_list.csv
	-./lab2_list --threads=8  --iterations=16  --yield=id --lists=4   >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=1   --yield=id --lists=4   >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=2   --yield=id --lists=4   >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=4   --yield=id --lists=4   >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=8   --yield=id --lists=4   >> lab2b_list.csv
	-./lab2_list --threads=12 --iterations=16  --yield=id --lists=4   >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=1   --yield=id --lists=4   >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=2   --yield=id --lists=4   >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=4   --yield=id --lists=4   >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=8   --yield=id --lists=4   >> lab2b_list.csv
	-./lab2_list --threads=16 --iterations=16  --yield=id --lists=4   >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=1   --yield=id --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=2   --yield=id --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=4   --yield=id --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=8   --yield=id --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=16  --yield=id --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1   --yield=id --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=2   --yield=id --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=4   --yield=id --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=8   --yield=id --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=16  --yield=id --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1   --yield=id --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=2   --yield=id --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=4   --yield=id --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=8   --yield=id --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=16  --yield=id --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1   --yield=id --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=2   --yield=id --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=4   --yield=id --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=8   --yield=id --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=16  --yield=id --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=1   --yield=id --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=2   --yield=id --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=4   --yield=id --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=8   --yield=id --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=16  --yield=id --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=1   --yield=id --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=2   --yield=id --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=4   --yield=id --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=8   --yield=id --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=16  --yield=id --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1   --yield=id --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=2   --yield=id --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=4   --yield=id --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=8   --yield=id --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=16  --yield=id --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1   --yield=id --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=2   --yield=id --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=4   --yield=id --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=8   --yield=id --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=16  --yield=id --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1   --yield=id --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=2   --yield=id --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=4   --yield=id --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=8   --yield=id --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=16  --yield=id --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=1   --yield=id --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=2   --yield=id --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=4   --yield=id --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=8   --yield=id --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=16 --iterations=16  --yield=id --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=1000  --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000  --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000  --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000  --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000  --lists=4  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=1000  --lists=8  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000  --lists=8  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000  --lists=8  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000  --lists=8  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000  --lists=8  --sync=m   >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=1000  --lists=16 --sync=m   >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000  --lists=16 --sync=m   >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000  --lists=16 --sync=m   >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000  --lists=16 --sync=m   >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000  --lists=16 --sync=m   >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=1000  --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000  --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000  --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000  --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000  --lists=4  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=1000  --lists=8  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000  --lists=8  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000  --lists=8  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000  --lists=8  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000  --lists=8  --sync=s   >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=1000  --lists=16 --sync=s   >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000  --lists=16 --sync=s   >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000  --lists=16 --sync=s   >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000  --lists=16 --sync=s   >> lab2b_list.csv
	./lab2_list --threads=12 --iterations=1000  --lists=16 --sync=s   >> lab2b_list.csv
