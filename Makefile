CC = mpicc
CFLAGS = -O2

all: 1-hello_mpi 2-matmat 2-matvec 3-process_topologies 4-jacobi

1-hello_mpi: 1-hello_mpi.c
	$(CC) $(CFLAGS) 1-hello_mpi.c -o 1-hello_mpi

2-matmat: 2-matmat.c
	$(CC) $(CFLAGS) 2-matmat.c -o 2-matmat

2-matvec: 2-matvec.c
	$(CC) $(CFLAGS) 2-matvec.c -o 2-matvec

3-process_topologies: 3-process_topologies.c
	$(CC) $(CFLAGS) 3-process_topologies.c -o 3-process_topologies

4-jacobi: 4-jacobi.c
	$(CC) $(CFLAGS) 4-jacobi.c -o 4-jacobi -lm

clean:
	rm -f 1-hello_mpi 2-matmat 2-matvec 3-process_topologies 4-jacobi
