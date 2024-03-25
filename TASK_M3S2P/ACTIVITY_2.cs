#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <mpi.h>

using namespace std;
using namespace std::chrono;

// FUNCTION TO GENERATE RANDOM VECTOR
void randomVector(int vector[], int size) {
    for (int i = 0; i < size; i++) {
        vector[i] = rand() % 100;
    }
}

int main(int argc, char **argv) {
    // INITIALIZE MPI
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // DEFINE VECTOR SIZE
    unsigned long vecSize = 100000000;
    int *v1, *v2, *v3;
    auto start = high_resolution_clock::now();

    if (rank == 0) {
        // MASTER PROCESS
        srand(time(0));
        // ALLOCATE MEMORY FOR VECTORS V1, V2, AND V3
        v1 = (int *)malloc(vecSize * sizeof(int));
        v2 = (int *)malloc(vecSize * sizeof(int));
        v3 = (int *)malloc(vecSize * sizeof(int));
        // GENERATE RANDOM VECTORS V1 AND V2
        randomVector(v1, vecSize);
        randomVector(v2, vecSize);
    }

    // ALLOCATE MEMORY FOR LOCAL VECTORS OF EACH PROCESS
    int *localV1 = (int *)malloc(vecSize / size * sizeof(int));
    int *localV2 = (int *)malloc(vecSize / size * sizeof(int));
    int *localV3 = (int *)malloc(vecSize / size * sizeof(int));

    // SCATTER DATA FROM MASTER TO ALL PROCESSES
    MPI_Scatter(v1, vecSize / size, MPI_INT, localV1, vecSize / size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(v2, vecSize / size, MPI_INT, localV2, vecSize / size, MPI_INT, 0, MPI_COMM_WORLD);

    // PERFORM LOCAL VECTOR ADDITION
    for (int i = 0; i < vecSize / size; i++) {
        localV3[i] = localV1[i] + localV2[i];
    }

    // GATHER RESULTS FROM ALL PROCESSES TO MASTER
    MPI_Gather(localV3, vecSize / size, MPI_INT, v3, vecSize / size, MPI_INT, 0, MPI_COMM_WORLD);

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);

    if (rank == 0) {
        // MASTER PROCESS
        cout << "Time taken by function: " << duration.count() << " microseconds" << endl;

        // CALCULATE TOTAL SUM OF ALL ELEMENTS IN V3 USING MPI_REDUCE
        int totalSum = 0;
        for (int i = 0; i < vecSize; i++) {
            totalSum += v3[i];
        }
        int globalSum;
        MPI_Reduce(&totalSum, &globalSum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        cout << "Total sum of all elements in v3: " << globalSum << endl;

        // FREE MEMORY
        free(v1);
        free(v2);
        free(v3);
    }

    // FREE LOCAL VECTORS
    free(localV1);
    free(localV2);
    free(localV3);

    // FINALIZE MPI
    MPI_Finalize();
    return 0;
}
