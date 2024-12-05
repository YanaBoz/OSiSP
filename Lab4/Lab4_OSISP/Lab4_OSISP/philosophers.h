#include <iostream>
#include <random>
#include <thread>
#include <vector>
#include <mutex>

using namespace std;

const int NUM_PHILOSOPHERS = 5;       // Количество философов
const int SIMULATION_TIME = 10000;    // Время моделирования в мс

enum ConflictResolution {
    SEMAPHORES,
    MONITORS,
    PRIORITIES
};

enum PhilosopherState {
    THINKING,
    EATING,
    WAITING
};

class Philosopher {
public:
    Philosopher(int id, ConflictResolution conflict_resolution);
    Philosopher(const Philosopher& other);
    void start();
    void think();
    void eat();
    void wait();

public:
    int id;                       // Идентификатор философа
    ConflictResolution conflict_resolution; // Логика разрешения конфликтов
    PhilosopherState state;        // Текущее состояние
    mutex mtx;                   // Мьютекс для синхронизации доступа к состоянию
    condition_variable cv;        // Условные переменные для синхронизации доступа к ресурсам
};

void print_state(const vector<Philosopher>& philosophers);