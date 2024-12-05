#include <iostream>
#include <random>
#include <thread>
#include <vector>
#include <mutex>

using namespace std;

const int NUM_PHILOSOPHERS = 5;       // ���������� ���������
const int SIMULATION_TIME = 10000;    // ����� ������������� � ��

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
    int id;                       // ������������� ��������
    ConflictResolution conflict_resolution; // ������ ���������� ����������
    PhilosopherState state;        // ������� ���������
    mutex mtx;                   // ������� ��� ������������� ������� � ���������
    condition_variable cv;        // �������� ���������� ��� ������������� ������� � ��������
};

void print_state(const vector<Philosopher>& philosophers);