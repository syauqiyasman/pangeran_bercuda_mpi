#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

void fill_matrix(double *mat, int rows, int cols) {
    for (int i = 0; i < rows * cols; i++)
        mat[i] = (double)(rand() % 100) / 10.0;
}

void fill_vector(double *vec, int n) {
    for (int i = 0; i < n; i++)
        vec[i] = (double)(rand() % 100) / 10.0;
}

int main(int argc, char *argv[]) {
    int rank, size;
    int N;
    double *matrix = NULL, *vector = NULL, *result = NULL;
    double *local_matrix, *local_result;
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
    local_matrix = (double *)malloc(rows_per_proc * N * sizeof(double));
    local_result = (double *)malloc(rows_per_proc * sizeof(double));
    vector = (double *)malloc(N * sizeof(double));

    if (rank == 0) {
        matrix = (double *)malloc(N * N * sizeof(double));
        result = (double *)malloc(N * sizeof(double));
        srand(42);
        fill_matrix(matrix, N, N);
        fill_vector(vector, N);
    }

    /* --- Communication: Scatter matrix rows & broadcast vector --- */
    t_comm_start = MPI_Wtime();
    MPI_Scatter(matrix, rows_per_proc * N, MPI_DOUBLE,
                local_matrix, rows_per_proc * N, MPI_DOUBLE,
                0, MPI_COMM_WORLD);
    MPI_Bcast(vector, N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    t_comm_end = MPI_Wtime();
    t_comm += (t_comm_end - t_comm_start);

    /* --- Computation: Local matrix-vector multiplication --- */
    t_comp_start = MPI_Wtime();
    for (int i = 0; i < rows_per_proc; i++) {
        local_result[i] = 0.0;
        for (int j = 0; j < N; j++) {
            local_result[i] += local_matrix[i * N + j] * vector[j];
        }
    }
    t_comp_end = MPI_Wtime();
    t_comp = (t_comp_end - t_comp_start);

    /* --- Communication: Gather results --- */
    t_comm_start = MPI_Wtime();
    MPI_Gather(local_result, rows_per_proc, MPI_DOUBLE,
               result, rows_per_proc, MPI_DOUBLE,
               0, MPI_COMM_WORLD);
    t_comm_end = MPI_Wtime();
    t_comm += (t_comm_end - t_comm_start);

    /* Collect timing from all processes */
    double max_comp, max_comm;
    MPI_Reduce(&t_comp, &max_comp, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&t_comm, &max_comm, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Matrix-Vector Multiplication: N=%d, NP=%d\n", N, size);
        printf("Computation time: %.6f sec\n", max_comp);
        printf("Communication time: %.6f sec\n", max_comm);
        printf("Total time: %.6f sec\n", max_comp + max_comm);
        printf("Result[0]=%.4f, Result[%d]=%.4f\n", result[0], N-1, result[N-1]);
        free(matrix);
        free(result);
    }

    free(local_matrix);
    free(local_result);
    free(vector);
    MPI_Finalize();
    return 0;
}
