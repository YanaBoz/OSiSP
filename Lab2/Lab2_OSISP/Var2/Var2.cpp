#include <windows.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <string>
#include <chrono>
#include <thread>

using namespace std;
//многопоточное
vector<int> read_file_async(const wstring& filename) {
    HANDLE fileHandle = CreateFile(
        filename.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (fileHandle == INVALID_HANDLE_VALUE) {
        cerr << "Error opening file : " << GetLastError() << endl;
        return {};
    }

    OVERLAPPED overlapped = {};
    overlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    DWORD bytesRead = 0;
    char buffer[4096];

    vector<int> data;
    while (true) {
        BOOL result = ReadFile(
            fileHandle,
            buffer,
            sizeof(buffer),
            &bytesRead,
            &overlapped
        );

        if (result == FALSE && GetLastError() != ERROR_IO_PENDING) {
            cerr << "Reading error : " << GetLastError() << endl;
            CloseHandle(fileHandle);
            CloseHandle(overlapped.hEvent);
            return {};
        }
        WaitForSingleObject(overlapped.hEvent, INFINITE);
        GetOverlappedResult(fileHandle, &overlapped, &bytesRead, TRUE);
        string strData(buffer, bytesRead);
        stringstream ss(strData);
        string numberStr;
        while (getline(ss, numberStr, ' ')) {
            try {
                int value = stoi(numberStr);
                data.push_back(value);
            }
            catch (const std::exception& e) {
                cerr << "Number parsing error : " << e.what() << endl;
            }
        }
        if (bytesRead < sizeof(buffer)) {
            break;
        }
    }
    CloseHandle(fileHandle);
    CloseHandle(overlapped.hEvent);

    return data;
}

void write_file_async(const wstring& filename, const vector<int>& data) {
    HANDLE fileHandle = CreateFile(
        filename.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_WRITE,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (fileHandle == INVALID_HANDLE_VALUE) {
        cerr << "Error opening file : " << GetLastError() << endl;
        return;
    }

    OVERLAPPED overlapped = {};
    overlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    DWORD bytesWritten = 0;
    string strData;
    for (int i = 0; i < data.size(); i++) {
        ostringstream oss;
        oss << data[i] << " ";
        strData += oss.str();
    }
    BOOL result = WriteFile(
        fileHandle,
        strData.c_str(),
        strData.size(),
        &bytesWritten,
        &overlapped
    );

    if (result == FALSE && GetLastError() != ERROR_IO_PENDING) {
        cerr << "Write error: " << GetLastError() << endl;
        CloseHandle(fileHandle);
        CloseHandle(overlapped.hEvent);
        return;
    }
    WaitForSingleObject(overlapped.hEvent, INFINITE);
    GetOverlappedResult(fileHandle, &overlapped, &bytesWritten, TRUE);
    CloseHandle(fileHandle);
    CloseHandle(overlapped.hEvent);
}
void sort_data_async(vector<int>& data) {
    sort(data.begin(), data.end());
}

int main() {
    wstring filename = L"data.txt";

    vector<int> numThreads = { 1, 2, 4, 8 };
    for (int threads : numThreads) {
        auto startTime = chrono::high_resolution_clock::now();
        vector<int> data = read_file_async(filename);

        vector<thread> threadPool;
        for (int i = 0; i < threads; ++i) {
            threadPool.push_back(thread(sort_data_async, ref(data)));
        }

        for (auto& thread : threadPool) {
            thread.join();
        }

        write_file_async(filename, data);
        auto endTime = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);

        cout << "Threads: " << threads << endl;
        cout << "Time taken: " << duration.count() << " milliseconds" << endl;
    }

    return 0;
}