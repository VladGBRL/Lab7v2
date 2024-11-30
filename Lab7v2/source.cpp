#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <mutex>
#include <queue>
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv;
int current_type = -1;             // Tipul curent (-1: niciun tip, 0: alb, 1: negru)
int active_processes = 0;    // Numărul de procese albe active
   // Numărul de procese negre active
std::queue<int> request_queue;     // Coada cererilor

void accessResource(pid_t pid, int type) {
    std::unique_lock<std::mutex> lock(mtx);

    // Adaugă cererea în coadă
    request_queue.push(type);

    // Așteaptă până când resursa devine disponibilă pentru acest tip
    cv.wait(lock, [&] {
        return (request_queue.front() == type && (current_type == type || current_type == -1));
        });

    // Procesul de același tip poate accesa resursa
    current_type = type;
    active_processes++;
    request_queue.pop();

    // Acces la resursă
    std::cout << "PID " << pid << " (tip " << (type == 0 ? "alb" : "negru")
        << ") accesează resursa.\n";

    lock.unlock();
    sleep(1); // Simulează timpul de utilizare a resursei
    lock.lock();

    std::cout << "PID " << pid << " (tip " << (type == 0 ? "alb" : "negru")
        << ") eliberează resursa.\n";

    active_processes--;

    // Dacă nu mai sunt procese active de același tip, permite accesul altui tip
    if (active_processes == 0) {
        current_type = -1; // Resursa devine disponibilă pentru alt tip
        
    }

        cv.notify_all();
}

void createChildProcess(int type) {

        
        accessResource(getpid(), type);
   
}

int main() {
    const int num_processes = 10;

    for (int i = 0; i < num_processes; ++i) {
        pid_t pid = fork();
        createChildProcess(i % 2); // Procesele sunt alternate între alb și negru
    }

    // Așteaptă finalizarea tuturor proceselor copil
    for (int i = 0; i < num_processes; ++i) {
        wait(nullptr);
    }

    return 0;
}
