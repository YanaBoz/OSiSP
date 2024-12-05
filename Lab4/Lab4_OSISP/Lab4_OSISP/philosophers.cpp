#include "philosophers.h"

Philosopher::Philosopher(int id, ConflictResolution conflict_resolution) :
    id(id),
    conflict_resolution(conflict_resolution),
    state(THINKING)
{
}

Philosopher::Philosopher(const Philosopher& other) :
    id(other.id),
    conflict_resolution(other.conflict_resolution),
    state(other.state),
    mtx(),
    cv()
{
}

void Philosopher::start() {
    thread t(&Philosopher::think, this);
    t.detach();
}

void Philosopher::think() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(1, 100);
    int think_time = dist(gen);

    this_thread::sleep_for(chrono::milliseconds(think_time));

    wait();
}

void Philosopher::eat() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(1, 100);
    int eat_time = dist(gen);

    this_thread::sleep_for(chrono::milliseconds(eat_time));

    think();
}

void Philosopher::wait() {
    unique_lock<mutex> lock(mtx);
    cv.wait(lock, [this] { return state == THINKING; });

    if (state == EATING) {
        return;
    }

    state = EATING;

    cv.notify_all();
}


void print_state(const vector<Philosopher>& philosophers) {
    cout << "Текущее состояние философов:" << endl;
    for (const Philosopher& philosopher : philosophers) {
        switch (philosopher.state) {
        case THINKING:
            cout << "Философ " << philosopher.id << " размышляет" << endl;
            break;
        case EATING:
            cout << "Философ " << philosopher.id << " ест" << endl;
            break;
        case WAITING:
            cout << "Философ " << philosopher.id << " ждет" << endl;
            break;
        }
    }
    cout << endl;
}

// Функция main()

int main() {
    setlocale(LC_ALL, "Russian");

    vector<Philosopher> philosophers;
    philosophers.emplace_back(0, SEMAPHORES);
    philosophers.emplace_back(1, MONITORS);
    philosophers.emplace_back(2, PRIORITIES);

    for (Philosopher& philosopher : philosophers) {
        philosopher.start();
    }

    while (true) {
        print_state(philosophers);
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    return 0;
}