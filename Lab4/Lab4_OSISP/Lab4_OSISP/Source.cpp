#include "philosophers.h"

int main() {
    // Создаем вектор философов с различными логиками разрешения конфликтов
    vector<Philosopher> philosophers;
    philosophers.emplace_back(0, SEMAPHORES);
    philosophers.emplace_back(1, MONITORS);
    philosophers.emplace_back(2, PRIORITIES);

    // Запускаем потоки философов
    for (Philosopher& philosopher : philosophers) {
        philosopher.start();
    }

    // Выводим состояние философов каждые 100 мс
    while (true) {
        print_state(philosophers);
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    return 0;
}
