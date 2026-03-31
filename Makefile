CC = mpicc
CFLAGS = -O2

all: 1-hello_mpi 2-matmat 2-matvec

hello_mpi:
	$(CC) $(CFLAGS) 1-hello_mpi.c -o 1-hello_mpi

matmat:
	$(CC) $(CFLAGS) 2-matmat.c -o 2-matmat

matvec:
	$(CC) $(CFLAGS) 2-matvec.c -o 2-matvec

clean:
	rm -f 1-hello_mpi 2-matmat 2-matvec
