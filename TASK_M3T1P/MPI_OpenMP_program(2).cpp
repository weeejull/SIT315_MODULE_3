#include <iostream>
#include <chrono>
#include <random>
#include <mpi.h>

using namespace std;
using namespace chrono;

const int MAX_SIZE = 10;

// FUNCTION TO GENERATE RANDOM VALUES FOR A MATRIX
void generateRandomMatrix(int matrix[][MAX_SIZE], int N) {
    random_device rd; // RANDOM DEVICE FOR SEED GENERATION
    mt19937 gen(rd()); // MERSERNE TWISTER ENGINE FOR RANDOM NUMBER GENERATION
    uniform_int_distribution<> dis(1, 10); // UNIFORM DISTRIBUTION FOR RANDOM NUMBERS BETWEEN 1 AND 10
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            matrix[i][j] = dis(gen); // ASSIGN RANDOM VALUES TO MATRIX ELEMENTS
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
    MPI_Init(&argc, &argv); // INITIALIZE MPI

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // GET RANK OF CURRENT PROCESS
    MPI_Comm_size(MPI_COMM_WORLD, &size); // GET TOTAL NUMBER OF PROCESSES

    int N;
    if (rank == 0) {
        cout << "Enter the size of the matrix (up to " << MAX_SIZE << "): ";
        cin >> N;

        // CHECK IF MATRIX SIZE IS WITHIN BOUNDS
        if (N <= 0 || N > MAX_SIZE) {
            cout << "Invalid matrix size." << endl;
            MPI_Abort(MPI_COMM_WORLD, 1); // ABORT MPI COMMUNICATION DUE TO INVALID SIZE
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
    MPI_Gather(C[startRow], (endRow - startRow) * N, MPI_INT, C, (endRow - startRow) * N, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        // PRINT EXECUTION TIME
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        cout << "Execution time using MPI and OpenMP: " << duration.count() << " microseconds" << endl;

        // PRINT THE MATRIX DIRECTLY TO THE TERMINAL
        cout << "Resulting Matrix:" << endl;
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                cout << C[i][j] << " ";
            }
            cout << endl;
        }
    }

    MPI_Finalize(); // FINALIZE MPI
    return 0;
}

