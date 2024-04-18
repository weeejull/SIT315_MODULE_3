#include <iostream>
#include <fstream>
#include <chrono>
#include <random>
#include <mpi.h>

using namespace std;
using namespace chrono;

const int MAX_SIZE = 10; // MAXIMUM SIZE OF THE MATRIX

// FUNCTION TO GENERATE RANDOM VALUES FOR A MATRIX
void generateRandomMatrix(int matrix[][MAX_SIZE], int N) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(1, 10);
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            matrix[i][j] = dis(gen);
        }
    }
}

// FUNCTION TO PERFORM MATRIX MULTIPLICATION FOR A SUBSET OF ROWS
void matrixMultiplication(int A[][MAX_SIZE], int B[][MAX_SIZE], int C[][MAX_SIZE], int startRow, int endRow, int N) {
    for (int i = startRow; i < endRow; ++i) {
        for (int j = 0; j < N; ++j) {
            C[i][j] = 0;
            for (int k = 0; k < N; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int N;
    if (rank == 0) {
        cout << "Enter the size of the matrix (up to " << MAX_SIZE << "): ";
        cin >> N;

        // CHECK IF MATRIX SIZE IS WITHIN BOUNDS
        if (N <= 0 || N > MAX_SIZE) {
            cout << "Invalid matrix size." << endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    // BROADCAST MATRIX SIZE TO ALL PROCESSES
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // CALCULATE ROWS PER PROCESS
    int rowsPerProcess = N / size;
    int startRow = rank * rowsPerProcess;
    int endRow = (rank == size - 1) ? N : (rank + 1) * rowsPerProcess;

    // ALLOCATE MEMORY FOR MATRICES A, B, AND C
    int A[MAX_SIZE][MAX_SIZE], B[MAX_SIZE][MAX_SIZE], C[MAX_SIZE][MAX_SIZE];

    // GENERATE RANDOM VALUES FOR MATRICES A AND B
    generateRandomMatrix(A, N);
    generateRandomMatrix(B, N);

    // PERFORM MATRIX MULTIPLICATION
    auto start = high_resolution_clock::now();
    matrixMultiplication(A, B, C, startRow, endRow, N);

    // GATHER RESULTS FROM ALL PROCESSES TO PROCESS 0
    MPI_Gather(&C[startRow][0], rowsPerProcess * N, MPI_INT, &C[0][0], rowsPerProcess * N, MPI_INT, 0, MPI_COMM_WORLD);

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);

    if (rank == 0) {
        // PRINT EXECUTION TIME
        cout << "Execution time using MPI: " << duration.count() << " microseconds" << endl;

        // PRINT THE MULTIPLIED MATRIX TO THE TERMINAL
        cout << "Multiplied Matrix:" << endl;
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                cout << C[i][j] << " ";
            }
            cout << endl;
        }
    }

    MPI_Finalize();
    return 0;
}

