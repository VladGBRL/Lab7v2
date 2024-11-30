#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <mutex>
#include <queue>
#include <condition_variable>

// Shared variables
std::mutex mtx;
std::condition_variable cv;
int current_type = -1;          // Current type (-1: none, 0: white, 1: black)
int active_processes = 0;       // Number of active processes of the current type
std::queue<int> request_queue;  // Request queue

// Access the shared resource
void accessResource(pid_t pid, int type) {
    std::unique_lock<std::mutex> lock(mtx);

    // Add the request to the queue
    request_queue.push(type);

    // Wait until the resource is available for this type
    cv.wait(lock, [&] {
        return (request_queue.front() == type &&
            (current_type == type || current_type == -1));
        });

    // Process of the same type can access the resource
    current_type = type;
    active_processes++;
    request_queue.pop();

    // Access the resource
    std::cout << "PID " << pid << " (type " << (type == 0 ? "white" : "black")
        << ") is accessing the resource.\n";

    lock.unlock();
    sleep(1);  // Simulate resource usage time
    lock.lock();

    std::cout << "PID " << pid << " (type " << (type == 0 ? "white" : "black")
        << ") is releasing the resource.\n";

    active_processes--;

    // If no more processes of the same type are active, allow other types
    if (active_processes == 0) {
        current_type = -1;  // Resource becomes available for another type
    }

    cv.notify_all();
}

// Create child process
void createChildProcess(int type) {
    pid_t pid = fork();

    if (pid == 0) {  // Child process
        accessResource(getpid(), type);
        _exit(0);  // Terminate the child process
    }
    else if (pid < 0) {
        std::cerr << "Failed to fork process\n";
    }
}

int main() {
    const int num_processes = 10;

    // Create child processes
    for (int i = 0; i < num_processes; ++i) {
        createChildProcess(i % 2);  // Alternate between white (0) and black (1)
    }

    // Wait for all child processes to complete
    for (int i = 0; i < num_processes; ++i) {
        wait(nullptr);
    }

    return 0;
}
