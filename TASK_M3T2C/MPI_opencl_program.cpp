#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <CL/cl.hpp>

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
    // Initialize input data
    const int MAX_ARRAY_SIZE = 300;
    vector<int> arr(MAX_ARRAY_SIZE);
    for (int i = MAX_ARRAY_SIZE; i > 0; --i)
    {
        arr.push_back(i);
    }

    // Start measuring execution time
    auto start = high_resolution_clock::now();

    // Perform quicksort
    quicksort(arr, 0, arr.size() - 1, MAX_ARRAY_SIZE);

    // Stop measuring execution time
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);

    // Print execution time
    cout << "Execution time: " << duration.count() << " microseconds" << endl;

    // Write the resulting array to a text file
    ofstream outFile("file.txt");
    if (outFile.is_open())
    {
        for (int i = 0; i < MAX_ARRAY_SIZE; i++)
            outFile << arr[i] << " ";
        outFile.close();
        cout << "Resulting array saved to 'file.txt'." << endl;
    }
    else
    {
        cerr << "Unable to open file for writing." << endl;
    }

    return 0;
}
