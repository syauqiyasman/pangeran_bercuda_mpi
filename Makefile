CC = mpicc
CFLAGS = -O2

all: 1-hello_mpi 2-matmat 2-matvec 3-process_topologies

1-hello_mpi:
	$(CC) $(CFLAGS) 1-hello_mpi.c -o 1-hello_mpi.o

2-matmat:
	$(CC) $(CFLAGS) 2-matmat.c -o 2-matmat.o

2-matvec:
	$(CC) $(CFLAGS) 2-matvec.c -o 2-matvec.o

topology_demo:
	$(CC) $(CFLAGS) 3-process_topologies.c -o 3-process_topologies.o -lm

hello_mpi: 1-hello_mpi
matmat: 2-matmat
matvec: 2-matvec
3-process_topologies: 3-process_topologies

clean:
	rm -f 1-hello_mpi 2-matmat 2-matvec 3-process_topologies
