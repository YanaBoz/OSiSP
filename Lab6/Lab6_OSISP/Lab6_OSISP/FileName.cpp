#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

// Получение всех ключей и значений из реестра
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

// Сохранение снимка реестра в файл
void SaveRegistrySnapshot(const vector<pair<wstring, wstring>>& keysAndValues, const wstring& filename) {
    wofstream file(filename);
    file << L"[HKEY_CURRENT_USER]\n";
    for (const auto& kv : keysAndValues) {
        file << L"\"" << kv.first << L"\"=\"" << kv.second << L"\"\n";
    }
    file.close();
}

// Сравнение текущего состояния с эталонным
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

// Отображение отличий
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

// Генерация reg-файла с изменениями
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

// Генерация инверсного reg-файла
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
    wstring path = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";  // Пример пути в реестре

    // Получение снимка текущего состояния реестра
    vector<pair<wstring, wstring>> baselineKeysAndValues = GetRegistryKeysAndValues(HKEY_CURRENT_USER, path);

    // Сохранение первого снимка
    SaveRegistrySnapshot(baselineKeysAndValues, L"baseline_snapshot.reg");

    // Ожидание изменений (например, задержка в 10 секунд)
    wcout << L"Waiting for changes...\n";
    Sleep(10000);  // Задержка на 10 секунд

    // Получение текущего состояния реестра после изменений
    vector<pair<wstring, wstring>> currentKeysAndValues = GetRegistryKeysAndValues(HKEY_CURRENT_USER, path);

    // Сравнение с базовым снимком
    vector<pair<wstring, wstring>> differences = CompareRegistrySnapshots(baselineKeysAndValues, currentKeysAndValues);

    // Отображение различий
    DisplayDifferences(differences);

    // Генерация reg-файла с изменениями
    GenerateRegFile(differences, L"registry_changes.reg");

    // Генерация инверсного reg-файла
    GenerateInverseRegFile(differences, L"registry_inverse_changes.reg");

    return 0;
}
