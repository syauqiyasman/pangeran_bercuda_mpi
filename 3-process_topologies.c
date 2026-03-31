#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void print_separator(int rank, const char* label) {
    if (rank == 0) {
        printf("\n========================================\n");
        printf("%s\n", label);
        printf("========================================\n");
        fflush(stdout);
        usleep(50000);
    }
    MPI_Barrier(MPI_COMM_WORLD);
}

int main(int argc, char *argv[]) {
    int my_rank, world_size;
    int ndims = 2;
    int dims[2]    = {0, 0};
    int periods[2] = {1, 1};
    int reorder    = 0;
    int coords[2];
    int source, dest;
    float a, b;

    MPI_Comm comm_2d;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    /* ================================================
     * TAHAP 1: MPI_Dims_create
     * ================================================ */
    print_separator(my_rank, "[TAHAP 1] MPI_Dims_create - Distribusi Proses Seimbang");

    MPI_Dims_create(world_size, ndims, dims);

    if (my_rank == 0) {
        printf("  Total proses  : %d\n", world_size);
        printf("  Grid otomatis : %d x %d\n", dims[0], dims[1]);
        fflush(stdout);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    /* ================================================
     * TAHAP 2: MPI_Cart_create + MPI_Cart_coords
     * ================================================ */
    print_separator(my_rank, "[TAHAP 2] MPI_Cart_create - Buat Topologi Grid 2D");

    MPI_Cart_create(MPI_COMM_WORLD, ndims, dims, periods, reorder, &comm_2d);
    MPI_Cart_coords(comm_2d, my_rank, ndims, coords);

    if (my_rank == 0) {
        printf("  Grid   : %d x %d (periodic di semua dimensi)\n", dims[0], dims[1]);
        printf("\n  %-6s | %-4s | %-4s\n", "Rank", "Row", "Col");
        printf("  -------|------|-----\n");
        fflush(stdout);
        usleep(10000);
    }
    MPI_Barrier(comm_2d);

    for (int r = 0; r < world_size; r++) {
        if (my_rank == r) {
            printf("  %-6d | %-4d | %-4d\n", my_rank, coords[0], coords[1]);
            fflush(stdout);
            usleep(5000);
        }
        MPI_Barrier(comm_2d);
    }

    /* ================================================
     * TAHAP 3: MPI_Cart_shift + MPI_Sendrecv
     * ================================================ */
    print_separator(my_rank, "[TAHAP 3] MPI_Cart_shift + MPI_Sendrecv - Pertukaran Data Tetangga");

    int displ = coords[1];
    MPI_Cart_shift(comm_2d, 0, displ, &source, &dest);

    a = (float)my_rank;
    MPI_Sendrecv(&a, 1, MPI_FLOAT, dest,   0,
                 &b, 1, MPI_FLOAT, source, 0,
                 comm_2d, &status);

    if (my_rank == 0) {
        printf("  Displacement = coords[1] (posisi kolom)\n");
        printf("\n  %-6s | %-8s | %-7s | %-6s | %-5s | %-5s\n",
               "Rank", "Coords", "Source", "Dest", "Sent", "Recv");
        printf("  -------|----------|---------|--------|-------|------\n");
        fflush(stdout);
        usleep(10000);
    }
    MPI_Barrier(comm_2d);

    for (int r = 0; r < world_size; r++) {
        if (my_rank == r) {
            printf("  %-6d | (%d,%d)     | %-7d | %-6d | %-5.0f | %-5.0f\n",
                   my_rank, coords[0], coords[1],
                   source, dest, a, b);
            fflush(stdout);
            usleep(5000);
        }
        MPI_Barrier(comm_2d);
    }

    /* ================================================
     * TAHAP 4: MPI_Cart_sub - Partisi grid per baris
     * ================================================ */
    print_separator(my_rank, "[TAHAP 4] MPI_Cart_sub - Partisi Grid Menjadi Subgrid Baris");

    int remain_dims[2] = {0, 1};
    MPI_Comm comm_row;
    MPI_Cart_sub(comm_2d, remain_dims, &comm_row);

    int row_rank, row_size;
    MPI_Comm_rank(comm_row, &row_rank);
    MPI_Comm_size(comm_row, &row_size);

    if (my_rank == 0) {
        printf("  remain_dims = (0,1) -> pertahankan dimensi kolom\n");
        printf("  Menghasilkan %d subgrid, masing-masing ukuran 1x%d\n\n",
               dims[0], dims[1]);
        printf("  %-6s | %-8s | %-10s | %-10s\n",
               "Rank", "Baris", "Row_Rank", "Row_Size");
        printf("  -------|----------|------------|------------\n");
        fflush(stdout);
        usleep(10000);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    for (int r = 0; r < world_size; r++) {
        if (my_rank == r) {
            printf("  %-6d | %-8d | %-10d | %-10d\n",
                   my_rank, coords[0], row_rank, row_size);
            fflush(stdout);
            usleep(5000);
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    /* ================================================
     * TAHAP 5: Benchmark Latency
     * ================================================ */
    print_separator(my_rank, "[TAHAP 5] Benchmark Latency MPI_Sendrecv");

    int bench_source, bench_dest;
    MPI_Cart_shift(comm_2d, 1, 1, &bench_source, &bench_dest);

    int iter = 1000;
    float send_val = (float)my_rank, recv_val;
    double total_time = 0.0;

    for (int i = 0; i < iter; i++) {
        double ts = MPI_Wtime();
        MPI_Sendrecv(&send_val, 1, MPI_FLOAT, bench_dest,   1,
                     &recv_val, 1, MPI_FLOAT, bench_source, 1,
                     comm_2d, &status);
        double te = MPI_Wtime();
        total_time += (te - ts);
    }

    double avg_time = (total_time / iter) * 1e6;
    double max_avg, min_avg;
    MPI_Reduce(&avg_time, &max_avg, 1, MPI_DOUBLE, MPI_MAX, 0, comm_2d);
    MPI_Reduce(&avg_time, &min_avg, 1, MPI_DOUBLE, MPI_MIN, 0, comm_2d);

    if (my_rank == 0) {
        printf("  Arah          : dimensi 1, displacement = 1\n");
        printf("  Iterasi       : %d\n", iter);
        printf("  Latency min   : %.3f µs\n", min_avg);
        printf("  Latency max   : %.3f µs\n", max_avg);
        printf("\n  Interpretasi:\n");
        printf("  >> Multi-core (sekarang) : ~1-5 µs   (shared memory)\n");
        printf("  >> Cluster (antar node)  : ~10-100 µs (jaringan)\n");
        fflush(stdout);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    /* ================================================
     * TAHAP 6: MPI_Cartdim_get + MPI_Cart_get
     * ================================================ */
    print_separator(my_rank, "[TAHAP 6] MPI_Cartdim_get + MPI_Cart_get - Inspeksi Topologi");

    if (my_rank == 0) {
        int ndims_get;
        int dims_get[2], periods_get[2], coords_get[2];

        MPI_Cartdim_get(comm_2d, &ndims_get);
        MPI_Cart_get(comm_2d, ndims_get, dims_get, periods_get, coords_get);

        printf("  Jumlah dimensi : %d\n",   ndims_get);
        printf("  Grid size      : %d x %d\n", dims_get[0], dims_get[1]);
        printf("  Periodic       : dim0=%d, dim1=%d\n",
               periods_get[0], periods_get[1]);
        printf("  Koordinat rank0: (%d, %d)\n",
               coords_get[0], coords_get[1]);
        fflush(stdout);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    if (my_rank == 0) {
        printf("\n========================================\n");
        printf("  Semua tahap selesai.\n");
        printf("========================================\n\n");
        fflush(stdout);
    }

    MPI_Comm_free(&comm_row);
    MPI_Comm_free(&comm_2d);
    MPI_Finalize();
    return 0;
}