#include <windows.h>
#include <iostream>
#include <vector>
 
CRITICAL_SECTION cs;
HANDLE hStartEvent;
HANDLE* hBlockedEvent = nullptr;
HANDLE* hResponseEvent = nullptr;
bool* bTerminate = nullptr;
std::vector<int>* marked = nullptr;

int* array = nullptr;
int arrSize = 0;
int markersCount = 0;

DWORD WINAPI MarkerThread(LPVOID pvParam) {
    int myNumber = *((int*)pvParam);
    int myIndex = myNumber - 1;
    WaitForSingleObject(hStartEvent, INFINITE);
    srand(myNumber);
    while (true)
    {
        int r = rand();
        int idx = r % arrSize;
        EnterCriticalSection(&cs);
        bool elementIsEmpty = (array[idx] == 0);

        if (elementIsEmpty)
        {
            Sleep(5);
            array[idx] = myNumber;
            marked[myIndex].push_back(idx);
            Sleep(5);
            LeaveCriticalSection(&cs);
            continue;
        }
        else
        {
            LeaveCriticalSection(&cs);
            std::cout << "Порядковый номер: " << myNumber << " | помечено элементов: " << marked[myIndex].size() << " | невозможно пометить: " << idx << std::endl;
            SetEvent(hBlockedEvent[myIndex]);
            WaitForSingleObject(hResponseEvent[myIndex], INFINITE);
            if (bTerminate[myIndex])
            {
                EnterCriticalSection(&cs);
                for (int i = 0; i < (int)marked[myIndex].size(); ++i)
                {
                    array[marked[myIndex][i]] = 0;
                }
                LeaveCriticalSection(&cs);
                return 0;
            }
        }
    }
}

int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    std::cout << "Введите размерность массива: ";
    std::cin >> arrSize;
    array = new int[arrSize];
    for (int i = 0; i < arrSize; ++i)
        array[i] = 0;
    std::cout << "Введите количество потоков marker: ";
    std::cin >> markersCount;
    InitializeCriticalSection(&cs);
    hStartEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    hBlockedEvent = new HANDLE[markersCount];
    hResponseEvent = new HANDLE[markersCount];
    bTerminate = new bool[markersCount];
    marked = new std::vector<int>[markersCount];

    for (int i = 0; i < markersCount; ++i)
    {
        hBlockedEvent[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        hResponseEvent[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        bTerminate[i] = false;
    }
    HANDLE* hThreads = new HANDLE[markersCount];
    int* numbers = new int[markersCount];
    DWORD* ids = new DWORD[markersCount];

    for (int i = 0; i < markersCount; ++i)
    {
        numbers[i] = i + 1;
        hThreads[i] = CreateThread(NULL, 0, MarkerThread, (LPVOID)&numbers[i], 0, &ids[i]);
    }
    SetEvent(hStartEvent);
    bool allFinished = false;
    while (!allFinished)
    {
        WaitForMultipleObjects(markersCount, hBlockedEvent, TRUE, INFINITE);
        EnterCriticalSection(&cs);
        std::cout << "\nСостояние массива\n";
        for (int i = 0; i < arrSize; ++i)
        {
            std::cout << array[i] << " ";
        }
        LeaveCriticalSection(&cs);

        int termNum;
        while (true)
        {
            std::cout << "Введите порядковый номер потока marker для завершения " << markersCount << "): ";
            std::cin >> termNum;
            if (termNum >= 1 && termNum <= markersCount)
                break;
            std::cout << "Некорректный номе!!\n";
        }

        int termIdx = termNum - 1;
        bTerminate[termIdx] = true;
        SetEvent(hResponseEvent[termIdx]);
        WaitForSingleObject(hThreads[termIdx], INFINITE);
        std::cout << "Marker " << termNum << "завершил работу!\n";

        EnterCriticalSection(&cs);
        std::cout << "\nСостояние массива после завершения: " << termNum;
        for (int i = 0; i < arrSize; ++i)
        {
            std::cout << array[i] << " ";
        }
        LeaveCriticalSection(&cs);

        for (int i = 0; i < markersCount; ++i)
        {
            if (i == termIdx)
                continue;
            bTerminate[i] = false;
            SetEvent(hResponseEvent[i]);
        }

        int activeCount = 0;
        for (int i = 0; i < markersCount; ++i)
        {
            DWORD exitCode;
            if (GetExitCodeThread(hThreads[i], &exitCode) && exitCode == STILL_ACTIVE)
            {
                activeCount++;
            }
        }
        if (activeCount == 0)
            allFinished = true;
    }

    for (int i = 0; i < markersCount; ++i)
    {
        CloseHandle(hThreads[i]);
        CloseHandle(hBlockedEvent[i]);
        CloseHandle(hResponseEvent[i]);
    }
    CloseHandle(hStartEvent);
    DeleteCriticalSection(&cs);

    delete[] array;
    delete[] hThreads;
    delete[] numbers;
    delete[] ids;
    delete[] hBlockedEvent;
    delete[] hResponseEvent;
    delete[] bTerminate;
    delete[] marked;

    std::cout << "Работа программы завершена!\n";
    return 0;
}