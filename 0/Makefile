#NAME: CHENCHEN
#EMAIL: chenchenstudent@gmail.com
#ID: 004710308

CC = gcc
CFLAGS = -g -Wall -Wextra -O0

.SILENT: check

default:
	$(CC) $(CFLAGS) -o lab0 lab0.c

check: check_one check_two check_three check_pass

check_one:
	./lab0 --ywwuyi;\
        if [ $$? -eq 1 ]; then echo "ywwuyi";\
	else echo "wuyikoei";\
        fi

check_two:
	./lab0 --input=NonExist.txt;\
        if [ $$? -eq 2 ]; then echo "ywwuyi";\
        else echo "wuyikoei";\
        fi

check_three:
	touch NON_CHANGE.txt;\
        chmod 444 NON_CHANGE.txt;\
        ./lab0 --output=NON_CHANGE.txt;\
        if [ $$? -eq 3 ]; then echo "ywwuyi";\
        else echo "wuyikoei";\
        fi

check_pass:
	echo "wuyikoei" > pass.txt;\
	./lab0 --input=pass.txt --output=out.txt;\
	if [ $$? -eq 0 ]; then echo "pass";\
	else echo "wuyikoei";\
	fi


clean:
	rm -f lab0 *.o *.txt *.tar.gz *~
dist:
	tar -czf  lab0-004710308.tar.gz lab0.c Makefile README breakpoint.png backtrace.png