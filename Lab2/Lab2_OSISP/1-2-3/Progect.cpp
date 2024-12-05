#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <chrono>

using namespace std;
//традиционный поток
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

    vector<int> data = read_file(filename);

    sort(data.begin(), data.end());

    write_file(filename, data);

    auto endTime = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);
    cout << "Data sorted in file" << endl;
    cout << "Time taken: " << duration.count() << " milliseconds" << endl;

    return 0;
}
