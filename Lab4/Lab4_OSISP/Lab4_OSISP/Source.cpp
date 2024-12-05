#include "philosophers.h"

int main() {
    // ������� ������ ��������� � ���������� �������� ���������� ����������
    vector<Philosopher> philosophers;
    philosophers.emplace_back(0, SEMAPHORES);
    philosophers.emplace_back(1, MONITORS);
    philosophers.emplace_back(2, PRIORITIES);

    // ��������� ������ ���������
    for (Philosopher& philosopher : philosophers) {
        philosopher.start();
    }

    // ������� ��������� ��������� ������ 100 ��
    while (true) {
        print_state(philosophers);
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    return 0;
}
