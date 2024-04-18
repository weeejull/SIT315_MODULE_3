#include <iostream>
#include <fstream>
#include <chrono>
#include <random>
#include <mpi.h>
#include <CL/cl.hpp>

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

    // INITIALIZE OpenCL
    cl::Context context(CL_DEVICE_TYPE_GPU); // CREATE OpenCL CONTEXT (CHANGE TO CL_DEVICE_TYPE_CPU IF GPU IS NOT AVAILABLE)
    vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>(); // GET OpenCL DEVICES
    cl::CommandQueue queue(context, devices[0]); // CREATE OpenCL COMMAND QUEUE
    ifstream sourceFile("matrix_multiplication.cl"); // OPEN OpenCL KERNEL FILE
    string sourceCode(istreambuf_iterator<char>(sourceFile), (istreambuf_iterator<char>())); // READ OpenCL KERNEL CODE
    cl::Program::Sources sources(1, make_pair(sourceCode.c_str(), sourceCode.length() + 1)); // CREATE OpenCL PROGRAM SOURCES
    cl::Program program(context, sources); // CREATE OpenCL PROGRAM
    program.build(devices); // BUILD OpenCL PROGRAM
    cl::Kernel kernel(program, "matrixMultiplication"); // CREATE OpenCL KERNEL

    // TRANSFER MATRICES A AND B TO DEVICE MEMORY
    cl::Buffer bufferA(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * N * N, A);
    cl::Buffer bufferB(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * N * N, B);
    cl::Buffer bufferC(context, CL_MEM_WRITE_ONLY, sizeof(int) * N * N);

    // SET KERNEL ARGUMENTS
    kernel.setArg(0, bufferA);
    kernel.setArg(1, bufferB);
    kernel.setArg(2, bufferC);
    kernel.setArg(3, N);
    kernel.setArg(4, startRow);
    kernel.setArg(5, endRow);

    // EXECUTE KERNEL
    cl::NDRange global(N);
    queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, cl::NullRange);

    // READ RESULT BACK TO HOST MEMORY
    queue.enqueueReadBuffer(bufferC, CL_TRUE, 0, sizeof(int) * N * N, C);

    auto start = high_resolution_clock::now(); // START TIMER
    auto stop = high_resolution_clock::now(); // STOP TIMER
    auto duration = duration_cast<microseconds>(stop - start); // CALCULATE DURATION

    if (rank == 0) {
        // PRINT EXECUTION TIME
        cout << "Execution time using MPI and OpenCL: " << duration.count() << " microseconds" << endl;

        // WRITE OUTPUT TO A FILE
        ofstream outputFile("output_mpi_opencl.txt");
        if (outputFile.is_open()) {
            for (int i = 0; i < N; ++i) {
                for (int j = 0; j < N; ++j) {
                    outputFile << C[i][j] << " ";
                }
                outputFile << endl;
            }
            outputFile.close();
            cout << "Output written to output_mpi_opencl.txt" << endl;
        } else {
            cerr << "Unable to open file for writing." << endl;
        }
    }

    MPI_Finalize(); // FINALIZE MPI
    return 0;
}

