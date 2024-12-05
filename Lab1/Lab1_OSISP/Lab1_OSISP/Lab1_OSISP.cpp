#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>

struct ApplicationContext {
    int counter = 0;
};

void saveContext(const ApplicationContext& context, const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (file.is_open()) {
        file.write(reinterpret_cast<const char*>(&context), sizeof(context));
        file.close();
    }
}

ApplicationContext loadContext(const std::string& filename) {
    ApplicationContext context;
    std::ifstream file(filename, std::ios::binary);
    if (file.is_open()) {
        file.read(reinterpret_cast<char*>(&context), sizeof(context));
        file.close();
    }
    return context;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_CLOSE) {
        ApplicationContext context;

        // Создание нового процесса
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        ZeroMemory(&pi, sizeof(pi));
        si.cb = sizeof(si);
        if (CreateProcessA(NULL, 
            (LPSTR)GetCommandLineA(),
            NULL,  
            NULL, 
            FALSE,  
            0,     
            NULL,  
            NULL,  
            &si,   
            &pi)) { // Информация о процессе
           
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        }
        else {

            MessageBoxA(hwnd, "Ошибка создания процесса", "Ошибка", MB_OK);
        }

        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

int main() {
    WNDCLASSEX wcex;
    ZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.hInstance = GetModuleHandle(NULL);
    wcex.lpszClassName = L"MyWindowClass";

    if (!RegisterClassEx(&wcex)) {
        MessageBoxA(NULL, "Ошибка регистрации класса окна", "Ошибка", MB_OK);
        return 1;
    }

    HWND hwnd = CreateWindowExA(0, "MyWindowClass",
        "Окно",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        300, 200,
        NULL, NULL,
        wcex.hInstance, NULL);
    if (!hwnd) {
        MessageBoxA(NULL, "Ошибка создания окна", "Ошибка", MB_OK);
        return 1;
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    ApplicationContext context = loadContext("context.dat");

    std::cout << "counter: " << context.counter << std::endl;
    context.counter++;
    saveContext(context, "context.dat");

    return 0;
}