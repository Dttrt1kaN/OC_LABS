#include <windows.h>
#include <iostream>
#include <cmath>

int* arr = nullptr;
int size = {};
int min_val = {};
int max_val = {};
double avg = {};

DWORD WINAPI MinMaxThread(LPVOID lpParam) {
    min_val = arr[0];
    max_val = arr[0];
    for (size_t i = 1; i < size; ++i) {
        if (arr[i] < min_val) {
            min_val = arr[i];
        }
        if (arr[i] > max_val) {
            max_val = arr[i];
        }
        Sleep(7);
    }
    std::cout << "Min: " << min_val << " Max: " << max_val << std::endl;
    return 0;
}

DWORD WINAPI AverageThread(LPVOID lpParam) {
    long long sum = 0;
    for (size_t i = 0; i < size; ++i) {
        sum += arr[i];
        Sleep(12);
    }

    avg = static_cast<double>(sum) / size;
    std::cout << "Average: " << avg << std::endl;
    return 0;
}

int main() {
    std::cout << "Please enter the array size: ";
    std::cin >> size;
    arr = new int[size];
    std::cout << "Please enter the array elements: ";
    for (size_t i = 0; i < size; ++i) {
        std::cin >> arr[i];
    }
    HANDLE hMinMax = CreateThread(NULL, 0, MinMaxThread, NULL, 0, NULL);
    HANDLE hAvg = CreateThread(NULL, 0, AverageThread, NULL, 0, NULL);
    if (hMinMax == NULL || hAvg == NULL) {
        std::cout << "Error creating threads." << std::endl;
        delete[] arr;
        return 1;
    }
    WaitForSingleObject(hMinMax, INFINITE);
    WaitForSingleObject(hAvg, INFINITE);
    int rounded_avg = static_cast<int>(std::round(avg));

    typedef void (*ReplaceMinMaxFunc)(int*, int, int, int, int);
    HMODULE hLib = LoadLibraryA("test.dll");
    if (hLib == NULL) {
        std::cout << "Error loading DLL!!" << std::endl;
        CloseHandle(hMinMax);
        CloseHandle(hAvg);
        delete[] arr;
        return 1;
    }

    ReplaceMinMaxFunc ReplaceMinMax = (ReplaceMinMaxFunc)GetProcAddress(hLib, "ReplaceMinMax");
    if (ReplaceMinMax == NULL) {
        std::cout << "Error getting function address from DLL!!" << std::endl;
        FreeLibrary(hLib);
        CloseHandle(hMinMax);
        CloseHandle(hAvg);
        delete[] arr;
        return 1;
    }
    ReplaceMinMax(arr, size, min_val, max_val, rounded_avg);
    FreeLibrary(hLib);

    std::cout << "\nFinal array:\n";
    for (size_t i = 0; i < size; ++i) {
        std::cout << arr[i] << " ";
    }
    std::cout << std::endl;
    CloseHandle(hMinMax);
    CloseHandle(hAvg);
    delete[] arr;
    return 0;
}