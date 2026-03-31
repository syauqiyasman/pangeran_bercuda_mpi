#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void fill_matrix(double *mat, int rows, int cols) {
    for (int i = 0; i < rows * cols; i++)
        mat[i] = (double)(rand() % 100) / 10.0;
}

int main(int argc, char *argv[]) {
    int rank, size;
    int N;
    double *A = NULL, *B = NULL, *C = NULL;
    double *local_A, *local_C;
    double t_comp_start, t_comp_end, t_comm_start, t_comm_end;
    double t_comp = 0.0, t_comm = 0.0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2) {
        if (rank == 0) printf("Usage: %s <matrix_size>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }
    N = atoi(argv[1]);

    if (N % size != 0) {
        if (rank == 0) printf("Matrix size %d not evenly divisible by %d processors\n", N, size);
        MPI_Finalize();
        return 1;
    }

    int rows_per_proc = N / size;
    local_A = (double *)malloc(rows_per_proc * N * sizeof(double));
    local_C = (double *)malloc(rows_per_proc * N * sizeof(double));
    B = (double *)malloc(N * N * sizeof(double));

    if (rank == 0) {
        A = (double *)malloc(N * N * sizeof(double));
        C = (double *)malloc(N * N * sizeof(double));
        srand(42);
        fill_matrix(A, N, N);
        fill_matrix(B, N, N);
    }

    /* --- Communication: Scatter A rows & broadcast B --- */
    t_comm_start = MPI_Wtime();
    MPI_Scatter(A, rows_per_proc * N, MPI_DOUBLE,
                local_A, rows_per_proc * N, MPI_DOUBLE,
                0, MPI_COMM_WORLD);
    MPI_Bcast(B, N * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    t_comm_end = MPI_Wtime();
    t_comm += (t_comm_end - t_comm_start);

    /* --- Computation: Local matrix-matrix multiplication --- */
    t_comp_start = MPI_Wtime();
    for (int i = 0; i < rows_per_proc; i++) {
        for (int j = 0; j < N; j++) {
            local_C[i * N + j] = 0.0;
            for (int k = 0; k < N; k++) {
                local_C[i * N + j] += local_A[i * N + k] * B[k * N + j];
            }
        }
    }
    t_comp_end = MPI_Wtime();
    t_comp = (t_comp_end - t_comp_start);

    /* --- Communication: Gather results --- */
    t_comm_start = MPI_Wtime();
    MPI_Gather(local_C, rows_per_proc * N, MPI_DOUBLE,
               C, rows_per_proc * N, MPI_DOUBLE,
               0, MPI_COMM_WORLD);
    t_comm_end = MPI_Wtime();
    t_comm += (t_comm_end - t_comm_start);

    double max_comp, max_comm;
    MPI_Reduce(&t_comp, &max_comp, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&t_comm, &max_comm, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Matrix-Matrix Multiplication: N=%d, NP=%d\n", N, size);
        printf("Computation time: %.6f sec\n", max_comp);
        printf("Communication time: %.6f sec\n", max_comm);
        printf("Total time: %.6f sec\n", max_comp + max_comm);
        printf("C[0][0]=%.4f, C[%d][%d]=%.4f\n", C[0], N-1, N-1, C[(N-1)*N+(N-1)]);
        free(A);
        free(C);
    }

    free(local_A);
    free(local_C);
    free(B);
    MPI_Finalize();
    return 0;
}
