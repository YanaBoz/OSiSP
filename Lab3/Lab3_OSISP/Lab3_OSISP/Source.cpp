#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>

struct Task {
    int id;
    std::vector<int> data;
    int result;
};

class TaskQueue {
public:
    std::mutex mtx;
    std::condition_variable cv;
    std::vector<Task> tasks;

    void addTask(const Task& task) {
        std::lock_guard<std::mutex> lock(mtx);
        tasks.push_back(task);
        cv.notify_one();
    }

    Task getTask() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&] { return !tasks.empty(); });
        Task task = tasks.front();
        tasks.erase(tasks.begin());
        return task;
    }
};

std::atomic<int> tasksRemaining;

std::atomic<bool> shouldExit;

void dispatcher(TaskQueue& taskQueue, std::vector<Task>& tasks, int numWorkers) {

    int tasksPerWorker = tasks.size() / numWorkers;

    for (int i = 0; i < numWorkers; i++) {
        std::vector<Task> workerTasks(tasks.begin() + i * tasksPerWorker, tasks.begin() + (i + 1) * tasksPerWorker);
        for (const Task& task : workerTasks) {
            taskQueue.addTask(task);
            tasksRemaining++;
        }
    }

    for (int i = numWorkers * tasksPerWorker; i < tasks.size(); i++) {
        taskQueue.addTask(tasks[i]);
        tasksRemaining++;
    }

    Task exitTask;
    exitTask.id = -1;
    for (int i = 0; i < numWorkers; i++) {
        taskQueue.addTask(exitTask);
    }
}

void worker(TaskQueue& taskQueue) {
    while (!shouldExit) {
        Task task = taskQueue.getTask();
        if (task.id == -1) {
            break;
        }

        task.result = 0;
        for (int x : task.data) {
            task.result += x * task.id;  
        }
        std::cout << "Обработана задача " << task.id << " с результатом " << task.result << std::endl;
        tasksRemaining--;
    }
}

int main() {
    setlocale(LC_ALL, "Russian");

    std::vector<Task> tasks;
    for (int i = 0; i < 10; i++) {
        Task task;
        task.id = i;
        task.data = { 1, 2, 3, 4, 5 };
        tasks.push_back(task);
    }

    TaskQueue taskQueue;

    int numWorkers = 4;

    tasksRemaining = tasks.size();
    shouldExit = false;

    std::thread dispatcherThread(dispatcher, std::ref(taskQueue), std::ref(tasks), numWorkers);

    std::vector<std::thread> workerThreads;
    for (int i = 0; i < numWorkers; i++) {
        workerThreads.push_back(std::thread(worker, std::ref(taskQueue)));
    }

    dispatcherThread.join();
    for (std::thread& t : workerThreads) {
        t.join();
    }

    return 0;
}