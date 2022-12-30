CFLAGS=-Wall -pedantic -g

main: main.o myio.o mybuffer.o mytests.o
	gcc -o main main.o myio.o mybuffer.o mytests.o

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $^

.PHONY: clean
clean:
	rm -f main.o myio.o mybuffer.o mytests.o outfile
