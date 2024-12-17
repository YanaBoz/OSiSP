#include <winsock2.h>
#include <iostream>
#include <fstream>

using namespace std; // ���������� ��������� using ��� ������������ ���� std

// IP-����� �������
#define IP_ADDRESS "127.0.0.1"

// ���� �������
#define PORT 54000

int main() {
    // ������������� �������
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cerr << "������ ������������� Winsock" << endl;
        return 1;
    }

    // �������� ������ �������
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "������ �������� ������ �������" << endl;
        return 1;
    }

    // ���������� ��������� sockaddr_in ��� ������ �������
    SOCKADDR_IN serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(IP_ADDRESS);

    // ����������� � �������
    if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "������ ����������� � �������" << endl;
        return 1;
    }

    // ��������� ������ �� ��������
    ifstream script("script.txt");
    string line;

    while (getline(script, line)) {
        // �������� ������� �������
        send(clientSocket, line.c_str(), line.length(), 0);

        // ��������� ���������� �� �������
        char result[1024];
        int resultLength = recv(clientSocket, result, sizeof(result), 0);
        if (resultLength <= 0) {
            cerr << "������ ��������� ���������� �� �������" << endl;
        }
        else {
            cout << "���������: " << result << endl;
        }
    }

    // �������� ������ �������
    closesocket(clientSocket);

    // ������������ �������� Winsock
    WSACleanup();

    return 0;
}
