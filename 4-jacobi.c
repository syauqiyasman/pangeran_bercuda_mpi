#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

/*
 * Parallel Jacobi Iteration for solving Ax = b
 *
 * Algorithm:
 *   x_i^(k+1) = (1/a_ii) * (b_i - sum(a_ij * x_j^(k), j != i))
 *
 * Matrix A is generated as diagonally dominant to guarantee convergence.
 * Row-wise distribution: each process gets N/NP rows.
 */

#define MAX_ITER 1000
#define EPSILON  1e-6

void generate_diag_dominant(double *A, double *b, int N) {
    for (int i = 0; i < N; i++) {
        double row_sum = 0.0;
        for (int j = 0; j < N; j++) {
            if (i != j) {
                A[i * N + j] = (double)(rand() % 100) / 100.0;
                row_sum += fabs(A[i * N + j]);
            }
        }
        /* Make strongly diagonal dominant for fast convergence */
        A[i * N + i] = row_sum * 2.0 + 1.0;
        b[i] = (double)(rand() % 100) / 10.0;
    }
}

int main(int argc, char *argv[]) {
    int rank, size;
    int N;
    double *A = NULL, *b = NULL;
    double *local_A, *local_b;
    double *x, *x_new, *local_x_new;
    double t_comp = 0.0, t_comm = 0.0;
    double t_start, t_end;

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
        if (rank == 0)
            printf("Matrix size %d not evenly divisible by %d processors\n", N, size);
        MPI_Finalize();
        return 1;
    }

    int rows_per_proc = N / size;

    /* Allocate local arrays */
    local_A = (double *)malloc(rows_per_proc * N * sizeof(double));
    local_b = (double *)malloc(rows_per_proc * sizeof(double));
    local_x_new = (double *)malloc(rows_per_proc * sizeof(double));
    x = (double *)calloc(N, sizeof(double));       /* current x (all zeros) */
    x_new = (double *)malloc(N * sizeof(double));   /* global x after gather */

    /* Root generates matrix */
    if (rank == 0) {
        A = (double *)malloc(N * N * sizeof(double));
        b = (double *)malloc(N * sizeof(double));
        srand(42);
        generate_diag_dominant(A, b, N);
    }

    /* --- Communication: Scatter A rows and b elements --- */
    t_start = MPI_Wtime();
    MPI_Scatter(A, rows_per_proc * N, MPI_DOUBLE,
                local_A, rows_per_proc * N, MPI_DOUBLE,
                0, MPI_COMM_WORLD);
    MPI_Scatter(b, rows_per_proc, MPI_DOUBLE,
                local_b, rows_per_proc, MPI_DOUBLE,
                0, MPI_COMM_WORLD);
    t_end = MPI_Wtime();
    t_comm += (t_end - t_start);

    int iter;
    int converged = 0;

    for (iter = 0; iter < MAX_ITER && !converged; iter++) {
        /* --- Computation: local Jacobi update --- */
        t_start = MPI_Wtime();
        double local_max_diff = 0.0;
        for (int i = 0; i < rows_per_proc; i++) {
            int global_i = rank * rows_per_proc + i;
            double sigma = 0.0;
            for (int j = 0; j < N; j++) {
                if (j != global_i) {
                    sigma += local_A[i * N + j] * x[j];
                }
            }
            local_x_new[i] = (local_b[i] - sigma) / local_A[i * N + global_i];

            double diff = fabs(local_x_new[i] - x[global_i]);
            if (diff > local_max_diff)
                local_max_diff = diff;
        }
        t_end = MPI_Wtime();
        t_comp += (t_end - t_start);

        /* --- Communication: gather updated x from all processes --- */
        t_start = MPI_Wtime();
        MPI_Allgather(local_x_new, rows_per_proc, MPI_DOUBLE,
                      x_new, rows_per_proc, MPI_DOUBLE,
                      MPI_COMM_WORLD);

        /* Check convergence across all processes */
        double global_max_diff;
        MPI_Allreduce(&local_max_diff, &global_max_diff, 1, MPI_DOUBLE,
                      MPI_MAX, MPI_COMM_WORLD);
        t_end = MPI_Wtime();
        t_comm += (t_end - t_start);

        /* Update x for next iteration */
        for (int i = 0; i < N; i++)
            x[i] = x_new[i];

        if (global_max_diff < EPSILON)
            converged = 1;
    }

    /* Collect max times across all processes */
    double max_comp, max_comm;
    MPI_Reduce(&t_comp, &max_comp, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&t_comm, &max_comm, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Jacobi Iteration: N=%d, NP=%d\n", N, size);
        printf("Computation time: %.6f sec\n", max_comp);
        printf("Communication time: %.6f sec\n", max_comm);
        printf("Total time: %.6f sec\n", max_comp + max_comm);
        printf("Iterations: %d, Converged: %s\n", iter, converged ? "YES" : "NO");

        /* Verify: compute residual ||Ax - b|| */
        double max_residual = 0.0;
        for (int i = 0; i < N; i++) {
            double ax_i = 0.0;
            for (int j = 0; j < N; j++)
                ax_i += A[i * N + j] * x[j];
            double r = fabs(ax_i - b[i]);
            if (r > max_residual)
                max_residual = r;
        }
        printf("Max residual ||Ax-b||_inf: %.6e\n", max_residual);
        printf("x[0]=%.6f, x[%d]=%.6f\n", x[0], N - 1, x[N - 1]);

        free(A);
        free(b);
    }

    free(local_A);
    free(local_b);
    free(local_x_new);
    free(x);
    free(x_new);
    MPI_Finalize();
    return 0;
}
