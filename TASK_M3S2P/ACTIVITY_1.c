#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        // MASTER PROCESS
        char message[] = "Hello World!";
        for (int dest = 1; dest < size; dest++) {
            MPI_Send(message, sizeof(message), MPI_CHAR, dest, 0, MPI_COMM_WORLD);
        }
    } else {
        // WORKER PROCESSES
        char message[100];
        MPI_Recv(message, 100, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Worker %d received message: %s\n", rank, message);
    }

    // BROADCASTING MESSAGES FROM MASTER TO WORKER
    if (rank == 0) {
        char broadcast_message[] = "Broadcasted Message!";
        MPI_Bcast(broadcast_message, sizeof(broadcast_message), MPI_CHAR, 0, MPI_COMM_WORLD);
    } else {
        char recv_broadcast_message[100];
        MPI_Bcast(recv_broadcast_message, 100, MPI_CHAR, 0, MPI_COMM_WORLD);
        printf("Worker %d received broadcasted message: %s\n", rank, recv_broadcast_message);
    }

    MPI_Finalize();
    return 0;
}
