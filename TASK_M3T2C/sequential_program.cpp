#include <iostream>
#include <vector>
#include <chrono>
#include <random>

using namespace std;
using namespace chrono;

// Partition function to divide the array into two parts
int partition(vector<int>& arr, int low, int high) {
    int pivot = arr[high];
    int i = low - 1;
    for (int j = low; j < high; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(arr[i], arr[j]);
        }
    }
    swap(arr[i + 1], arr[high]);
    return i + 1;
}

// Quicksort function
void quicksort(vector<int>& arr, int low, int high, int max_array_size) {
    if (low < high) {
        if (arr.size() > max_array_size) {
            cout << "Maximum array size reached. Sorting stopped." << endl;
            return;
        }
        int pi = partition(arr, low, high);
        quicksort(arr, low, pi - 1, max_array_size);
        quicksort(arr, pi + 1, high, max_array_size);
    }
}

int main() {
    // Reduced array size
    const int MAX_ARRAY_SIZE = 300; // Adjust as needed

    // Generate array
    vector<int> arr;
    arr.reserve(MAX_ARRAY_SIZE); // Preallocate memory
    for (int i = MAX_ARRAY_SIZE; i > 0; --i) {
        arr.push_back(i);
    }

    // Start measuring execution time
    auto start = high_resolution_clock::now();

    // Perform quicksort
    quicksort(arr, 0, arr.size() - 1, MAX_ARRAY_SIZE);

    // Stop measuring execution time
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);

    // Print sorted array (if sorting completed)
    if (arr.size() <= MAX_ARRAY_SIZE) {
        cout << "Sorted array: \n\n";
        for (int i = 0; i < arr.size(); i++)
            cout << arr[i] << " ";
        cout << endl;
    }

    // Print execution time
    cout << "\nExecution time: " << duration.count() << " microseconds" << endl;

    return 0;
}
