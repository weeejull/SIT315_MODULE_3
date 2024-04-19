#include <iostream>
#include <vector>
#include <chrono>
#include <mpi.h>
#include <fstream>

using namespace std;
using namespace chrono;

// Partition function to divide the array into two parts
int partition(vector<int> &arr, int low, int high)
{
    int pivot = arr[high];
    int i = low - 1;
    for (int j = low; j < high; j++)
    {
        if (arr[j] < pivot)
        {
            i++;
            swap(arr[i], arr[j]);
        }
    }
    swap(arr[i + 1], arr[high]);
    return i + 1;
}

// Quicksort function
void quicksort(vector<int> &arr, int low, int high, int max_array_size)
{
    if (low < high)
    {
        if (arr.size() > max_array_size)
        {
            cout << "Maximum array size reached. Sorting stopped." << endl;
            return;
        }
        int pi = partition(arr, low, high);
        quicksort(arr, low, pi - 1, max_array_size);
        quicksort(arr, pi + 1, high, max_array_size);
    }
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Reduced array size
    const int MAX_ARRAY_SIZE = 300;

    // Generate array
    vector<int> arr;
    if (world_rank == 0)
    {
        arr.reserve(MAX_ARRAY_SIZE); // Preallocate memory
        for (int i = MAX_ARRAY_SIZE; i > 0; --i)
        {
            arr.push_back(i);
        }
    }

    // Broadcast array size to all processes
    int temp_max_array_size = MAX_ARRAY_SIZE;
    MPI_Bcast(&temp_max_array_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Scatter array to all processes
    vector<int> local_arr(MAX_ARRAY_SIZE / world_size);
    MPI_Scatter(arr.data(), MAX_ARRAY_SIZE / world_size, MPI_INT, local_arr.data(), MAX_ARRAY_SIZE / world_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Start measuring execution time
    auto start = high_resolution_clock::now();

    // Perform quicksort locally
    quicksort(local_arr, 0, local_arr.size() - 1, MAX_ARRAY_SIZE);

    // Gather sorted subarrays
    vector<int> sorted_arr(MAX_ARRAY_SIZE);
    MPI_Gather(local_arr.data(), MAX_ARRAY_SIZE / world_size, MPI_INT, sorted_arr.data(), MAX_ARRAY_SIZE / world_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Stop measuring execution time
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);

    // Print execution time (only process 0)
    if (world_rank == 0)
    {
        cout << "Execution time: " << duration.count() << " microseconds" << endl;

        // Write the resulting array to a text file
        ofstream outFile("file.txt");
        if (outFile.is_open())
        {
            for (int i = 0; i < MAX_ARRAY_SIZE; i++)
                outFile << sorted_arr[i] << " ";
            outFile.close();
            cout << "Resulting array saved to 'file.txt'." << endl;
        }
        else
        {
            cerr << "Unable to open file for writing." << endl;
        }
    }

    MPI_Finalize();
    return 0;
}
