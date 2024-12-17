#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

// ��������� ���� ������ � �������� �� �������
vector<pair<wstring, wstring>> GetRegistryKeysAndValues(HKEY hKey, const wstring& subkey) {
    vector<pair<wstring, wstring>> keysAndValues;
    HKEY hSubKey;
    LONG result = RegOpenKeyEx(hKey, subkey.c_str(), 0, KEY_READ, &hSubKey);

    if (result == ERROR_SUCCESS) {
        DWORD index = 0;
        DWORD nameSize = 1024;
        DWORD dataSize = 1024;
        wchar_t nameBuffer[1024];
        wchar_t dataBuffer[1024];

        while (RegEnumValue(hSubKey, index, nameBuffer, &nameSize, NULL, NULL, (BYTE*)dataBuffer, &dataSize) == ERROR_SUCCESS) {
            wstring key(nameBuffer, nameSize);
            wstring value(dataBuffer, dataSize / sizeof(wchar_t));
            keysAndValues.push_back(make_pair(key, value));
            index++;
        }
        RegCloseKey(hSubKey);
    }

    return keysAndValues;
}

// ���������� ������ ������� � ����
void SaveRegistrySnapshot(const vector<pair<wstring, wstring>>& keysAndValues, const wstring& filename) {
    wofstream file(filename);
    file << L"[HKEY_CURRENT_USER]\n";
    for (const auto& kv : keysAndValues) {
        file << L"\"" << kv.first << L"\"=\"" << kv.second << L"\"\n";
    }
    file.close();
}

// ��������� �������� ��������� � ���������
vector<pair<wstring, wstring>> CompareRegistrySnapshots(
    const vector<pair<wstring, wstring>>& baseline,
    const vector<pair<wstring, wstring>>& current) {

    vector<pair<wstring, wstring>> differences;

    for (const auto& baselineKeyValue : baseline) {
        auto it = find_if(current.begin(), current.end(), [&baselineKeyValue](const pair<wstring, wstring>& currentKeyValue) {
            return currentKeyValue.first == baselineKeyValue.first;
            });

        if (it == current.end()) {
            differences.push_back(make_pair(baselineKeyValue.first, L"Deleted"));
        }
        else if (it->second != baselineKeyValue.second) {
            differences.push_back(make_pair(baselineKeyValue.first, it->second));
        }
    }

    return differences;
}

// ����������� �������
void DisplayDifferences(const vector<pair<wstring, wstring>>& differences) {
    if (differences.empty()) {
        wcout << L"No changes detected in the registry.\n";
    }
    else {
        wcout << L"Detected the following changes:\n";
        for (const auto& diff : differences) {
            wcout << L"Key: " << diff.first << L" => " << diff.second << endl;
        }
    }
}

// ��������� reg-����� � �����������
void GenerateRegFile(const vector<pair<wstring, wstring>>& differences, const wstring& filename) {
    wofstream file(filename);
    for (const auto& diff : differences) {
        if (diff.second == L"Deleted") {
            file << L"[-" << diff.first << L"]\n";
        }
        else {
            file << L"[" << diff.first << L"]\n";
            file << L"\"" << diff.second << L"\"\n";
        }
    }
    file.close();
}

// ��������� ���������� reg-�����
void GenerateInverseRegFile(const vector<pair<wstring, wstring>>& differences, const wstring& filename) {
    wofstream file(filename);
    for (const auto& diff : differences) {
        if (diff.second == L"Deleted") {
            file << L"[" << diff.first << L"]\n";
            file << L"\"" << diff.second << L"\"\n";
        }
        else {
            file << L"[-" << diff.first << L"]\n";
        }
    }
    file.close();
}

int main() {
    wstring path = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";  // ������ ���� � �������

    // ��������� ������ �������� ��������� �������
    vector<pair<wstring, wstring>> baselineKeysAndValues = GetRegistryKeysAndValues(HKEY_CURRENT_USER, path);

    // ���������� ������� ������
    SaveRegistrySnapshot(baselineKeysAndValues, L"baseline_snapshot.reg");

    // �������� ��������� (��������, �������� � 10 ������)
    wcout << L"Waiting for changes...\n";
    Sleep(10000);  // �������� �� 10 ������

    // ��������� �������� ��������� ������� ����� ���������
    vector<pair<wstring, wstring>> currentKeysAndValues = GetRegistryKeysAndValues(HKEY_CURRENT_USER, path);

    // ��������� � ������� �������
    vector<pair<wstring, wstring>> differences = CompareRegistrySnapshots(baselineKeysAndValues, currentKeysAndValues);

    // ����������� ��������
    DisplayDifferences(differences);

    // ��������� reg-����� � �����������
    GenerateRegFile(differences, L"registry_changes.reg");

    // ��������� ���������� reg-�����
    GenerateInverseRegFile(differences, L"registry_inverse_changes.reg");

    return 0;
}
