#include <winsock2.h>
#include <iostream>
#include <fstream>

using namespace std; // Добавление оператора using для пространства имен std

// IP-адрес сервера
#define IP_ADDRESS "127.0.0.1"

// Порт сервера
#define PORT 54000

int main() {
    // Инициализация сокетов
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cerr << "Ошибка инициализации Winsock" << endl;
        return 1;
    }

    // Создание сокета клиента
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "Ошибка создания сокета клиента" << endl;
        return 1;
    }

    // Заполнение структуры sockaddr_in для сокета клиента
    SOCKADDR_IN serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(IP_ADDRESS);

    // Подключение к серверу
    if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Ошибка подключения к серверу" << endl;
        return 1;
    }

    // Получение команд из сценария
    ifstream script("script.txt");
    string line;

    while (getline(script, line)) {
        // Отправка команды серверу
        send(clientSocket, line.c_str(), line.length(), 0);

        // Получение результата от сервера
        char result[1024];
        int resultLength = recv(clientSocket, result, sizeof(result), 0);
        if (resultLength <= 0) {
            cerr << "Ошибка получения результата от сервера" << endl;
        }
        else {
            cout << "Результат: " << result << endl;
        }
    }

    // Закрытие сокета клиента
    closesocket(clientSocket);

    // Освобождение ресурсов Winsock
    WSACleanup();

    return 0;
}
