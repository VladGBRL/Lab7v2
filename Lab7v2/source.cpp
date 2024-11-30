#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <mutex>
#include <queue>
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv;
int current_type = -1;          
int active_processes = 0;  
std::queue<int> request_queue;   

void accessResource(pid_t pid, int type) {
    std::unique_lock<std::mutex> lock(mtx);

    request_queue.push(type);

    cv.wait(lock, [&] {
        return (request_queue.front() == type && (current_type == type || current_type == -1));
        });

    current_type = type;
    active_processes++;
    request_queue.pop();

    std::cout << "PID " << pid << " (tip " << (type == 0 ? "alb" : "negru")
        << ") accesează resursa.\n";

    lock.unlock();
    sleep(1);i
    lock.lock();

    std::cout << "PID " << pid << " (tip " << (type == 0 ? "alb" : "negru")
        << ") eliberează resursa.\n";

    active_processes--;

    if (active_processes == 0) {
        current_type = -1; 
        
    }

        cv.notify_all();
}

void createChildProcess(int pid, int type) {

        
        accessResource(getpid(), type);
   
}

int main() {
    const int num_processes = 10;
    for (int i = 0; i < num_processes; ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            createChildProcess(getpid(), i % 2);
            exit(0); // Procesul copil se termină după ce accesează resursa.
        }
    }

    
    for (int i = 0; i < num_processes; ++i) {
        wait(nullptr);
    }

    return 0;
}
