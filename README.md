# MPI Parallel Statistics Program

This project is a C program that uses **MPI (Message Passing Interface)** to compute statistics and transformations on a large array of integers in parallel across multiple processes.

---

## Features

The program performs the following tasks in parallel:

1. **Generate a random array of integers** (by the root process).
2. **Compute global statistics**:
   - Maximum value
   - Minimum value
   - Average
   - Count of numbers above and below the average
   - Variance
3. **Normalize the array** to a percentage scale (0–100) into a `D` array.
4. **Compute prefix sums** across all elements.
5. Handles **uneven distribution** of numbers among processes.
6. Supports **multiple iterations** with user input to continue or exit.

---

## Requirements

- **C compiler** (e.g., `gcc`)
- **MPI library** (e.g., `OpenMPI`, `MPICH`)

---

## Compilation

```bash
mpicc -o mpi_stats mpi_stats.c

Execution

Run the program with multiple processes:

mpirun -np <num_processes> ./mpi_stats


Replace <num_processes> with the number of parallel processes you want to use.

Usage

The root process will ask for the total number of integers to generate.

The program will compute and print:

Global maximum and minimum

Global average

Count of numbers above and below the average

Variance

Normalized D array

Prefix sums

After displaying results, it will ask whether to continue (1) or exit (0).

Notes

The program dynamically distributes the numbers among the MPI processes.

All memory allocations are handled per iteration to avoid memory leaks.

Works with both even and uneven number of integers relative to the number of processes.

Author

Georgia Papapanagiotou