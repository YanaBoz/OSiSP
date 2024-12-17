#include <iostream>
#include <fstream>
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <string>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")
// IP-адрес сервера+
#define IP_ADDRESS "127.0.0.1"

// Порт сервера
#define PORT 54000

int main() {
    setlocale(LC_ALL, "Russian");
    // Инициализация сокетов
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "Ошибка инициализации Winsock" << std::endl;
        return 1;
    }

    // Создание сокета клиента
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Ошибка создания сокета клиента" << std::endl;
        return 1;
    }

    // Заполнение структуры sockaddr_in для сокета клиента
    SOCKADDR_IN serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, IP_ADDRESS, &serverAddr.sin_addr.s_addr);

    // Подключение к серверу
    if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Ошибка подключения к серверу" << std::endl;
        return 1;
    }

    // Получение команд из сценария
    std::ifstream script("script.txt");
    std::string line;
    while (std::getline(script, line)) {
        // Отправка команды серверу
        // Удаляем лишние символы в конце строки
        line.erase(std::remove_if(line.begin(), line.end(), [](unsigned char c) { return std::isspace(c); }), line.end());
        send(clientSocket, line.c_str(), line.length(), 0);

        // Получение результата от сервера
        char result[1024];
        int resultLength = recv(clientSocket, result, sizeof(result), 0);
        if (resultLength <= 0) {
            std::cerr << "Ошибка получения результата от сервера" << std::endl;
        }
        else {
            // Удаляем лишние символы в конце строки
            std::string resultString(result, resultLength);
            resultString.erase(std::remove_if(resultString.begin(), resultString.end(), [](unsigned char c) { return std::isspace(c); }), resultString.end());
            std::cout << "Результат: " << resultString << std::endl;
        }
    }

    // Закрытие сокета клиента
    closesocket(clientSocket);

    // Освобождение ресурсов Winsock
    WSACleanup();

    return 0;
}
