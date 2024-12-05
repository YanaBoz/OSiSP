#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <future>

using namespace std;
//асинхронное чтение-обработка-запись
vector<int> read_file(const string& filename) {
    vector<int> data;
    ifstream file(filename);
    if (file.is_open()) {
        int value;
        while (file >> value) {
            data.push_back(value);
        }
        file.close();
    }
    else {
        cerr << "Error opening file: " << filename << endl;
    }
    return data;
}

void write_file(const string& filename, const vector<int>& data) {
    ofstream file(filename);
    if (file.is_open()) {
        for (int i = 0; i < data.size(); ++i) {
            file << data[i] << " ";
        }
        file.close();
    }
    else {
        cerr << "Error opening file: " << filename << endl;
    }
}

int main() {
    string filename = "data.txt";

    auto startTime = chrono::high_resolution_clock::now();

    auto readFuture = async(launch::async, read_file, filename);

    readFuture.wait();

    auto sortedData = readFuture.get();
    sort(sortedData.begin(), sortedData.end());

    auto writeFuture = async(launch::async, write_file, filename, sortedData);

    writeFuture.wait();

    auto endTime = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);
    cout << "Data sorted in file" << endl;
    cout << "Time taken: " << duration.count() << " milliseconds" << endl;

    return 0;
}