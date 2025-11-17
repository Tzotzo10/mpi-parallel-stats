#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "mpi.h"

int main(int argc, char** argv) {
    int rank, size;
    int *nums = NULL;       // full array (root only)
    int *local_nums = NULL; // each process's portion
    int *sendcounts = NULL, *displs = NULL;
    int *D = NULL;
    int *prefix = NULL;
    int i;
    int N, local_count;
    int ans = 1; // continue flag

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    while (ans != 0) {

        if (rank == 0) {
            printf("Enter total number of integers: ");
            scanf("%d", &N);

            // Allocate and generate random numbers
            nums = (int*)malloc(N * sizeof(int));
            srand(time(NULL));
            for (i = 0; i < N; i++) {
                nums[i] = rand() % 100; // random numbers 0-99
            }
        }

        // Determine local counts
        int base_count = N / size;
        int remainder = N % size;
        local_count = base_count + (rank == size - 1 ? remainder : 0);
        local_nums = (int*)malloc(local_count * sizeof(int));

        // Prepare sendcounts and displs
        if (rank == 0) {
            sendcounts = (int*)malloc(size * sizeof(int));
            displs = (int*)malloc(size * sizeof(int));
            for (i = 0; i < size; i++) {
                sendcounts[i] = base_count + (i == size - 1 ? remainder : 0);
            }
            displs[0] = 0;
            for (i = 1; i < size; i++) {
                displs[i] = displs[i - 1] + sendcounts[i - 1];
            }
        }

        // Scatter the numbers
        MPI_Scatterv(nums, sendcounts, displs, MPI_INT,
                     local_nums, local_count, MPI_INT, 0, MPI_COMM_WORLD);

        // Local sum, min, max
        int local_sum = 0, local_max = local_nums[0], local_min = local_nums[0];
        for (i = 0; i < local_count; i++) {
            local_sum += local_nums[i];
            if (local_nums[i] > local_max) local_max = local_nums[i];
            if (local_nums[i] < local_min) local_min = local_nums[i];
        }

        int global_sum, global_max, global_min;
        MPI_Reduce(&local_sum, &global_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce(&local_max, &global_max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
        MPI_Reduce(&local_min, &global_min, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);

        float global_avg = 0;
        if (rank == 0) {
            global_avg = (float)global_sum / N;
            printf("Global max: %d\n", global_max);
            printf("Global min: %d\n", global_min);
            printf("Global average: %.2f\n", global_avg);
        }

        MPI_Bcast(&global_avg, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);

        // Count numbers above/below average
        int local_above = 0, local_below = 0;
        for (i = 0; i < local_count; i++) {
            if (local_nums[i] > global_avg) local_above++;
            if (local_nums[i] < global_avg) local_below++;
        }

        int global_above, global_below;
        MPI_Reduce(&local_above, &global_above, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce(&local_below, &global_below, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        if (rank == 0) {
            printf("%d numbers above average\n", global_above);
            printf("%d numbers below average\n", global_below);
        }

        // Variance
        double local_var_sum = 0;
        for (i = 0; i < local_count; i++) {
            local_var_sum += (local_nums[i] - global_avg) * (local_nums[i] - global_avg);
        }
        double global_var_sum;
        MPI_Reduce(&local_var_sum, &global_var_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

        if (rank == 0) {
            double variance = global_var_sum / N;
            printf("Variance: %.2f\n", variance);
        }

        // Normalized D array
        for (i = 0; i < local_count; i++) {
            local_nums[i] = (int)(((float)(local_nums[i] - global_min) / (global_max - global_min)) * 100);
        }

        if (rank == 0) D = (int*)malloc(N * sizeof(int));

        MPI_Gatherv(local_nums, local_count, MPI_INT,
                    D, sendcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);

        if (rank == 0) {
            int max_D_val = D[0], max_D_idx = 0;
            printf("Normalized D array:\n");
            for (i = 0; i < N; i++) {
                printf("%d ", D[i]);
                if (D[i] > max_D_val) {
                    max_D_val = D[i];
                    max_D_idx = i;
                }
            }
            printf("\nMax value in D: %d at index %d\n", max_D_val, max_D_idx);
        }

        // Prefix sums across all processes
        prefix = (int*)malloc(local_count * sizeof(int));
        MPI_Scan(local_nums, prefix, local_count, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

        if (rank == 0) {
            printf("Prefix sums (all elements):\n");
            int offset = 0;
            for (i = 0; i < size; i++) {
                int count = (i == size - 1 ? base_count + remainder : base_count);
                int j;
                for (j = 0; j < count; j++) {
                    printf("%d ", prefix[j]);
                }
                offset += count;
            }
            printf("\n");
        }

        // Ask user if they want to continue
        if (rank == 0) {
            printf("Enter 1 to continue, 0 to exit: ");
            scanf("%d", &ans);
        }
        MPI_Bcast(&ans, 1, MPI_INT, 0, MPI_COMM_WORLD);

        free(local_nums);
        free(prefix);
        if (rank == 0) {
            free(nums);
            free(sendcounts);
            free(displs);
            free(D);
        }
    }

    MPI_Finalize();
    return 0;
}