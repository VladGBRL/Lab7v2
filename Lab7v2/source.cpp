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
int active_processes = 0;          // Numărul de procese active
int active_white_processes = 0;    // Numărul de procese albe active
int active_black_processes = 0;    // Numărul de procese negre active
std::queue<int> request_queue;     // Coada cererilor

void accessResource(pid_t pid, int type) {
    std::unique_lock<std::mutex> lock(mtx);

    // Adaugă cererea în coadă
    request_queue.push(type);

    // Așteaptă până când resursa devine disponibilă pentru acest tip
    cv.wait(lock, [&] {
        // Condiție de acces: doar firele de același tip pot accesa simultan
        bool same_type_access = (current_type == -1 || current_type == type);
        bool no_conflict = (active_white_processes == 0 || type == 1) &&
            (active_black_processes == 0 || type == 0);
        bool is_turn = !request_queue.empty() && request_queue.front() == type;
        return same_type_access && no_conflict && is_turn;
        });

    // Procesul de același tip poate accesa resursa
    current_type = type;
    active_processes++;
    if (type == 0) {
        active_white_processes++;
    }
    else {
        active_black_processes++;
    }
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
    if (type == 0) {
        active_white_processes--;
    }
    else {
        active_black_processes--;
    }

    // Dacă nu mai sunt procese active de același tip, resursa devine disponibilă pentru alt tip
    if (active_white_processes == 0 && active_black_processes == 0) {
        current_type = -1;
    }

    cv.notify_all();
}

void createChildProcess(int type) {
    pid_t pid = fork();

    if (pid == 0) {
        // Cod pentru procesul copil
        accessResource(getpid(), type);
        exit(0); // Termină procesul copil
    }
    else if (pid < 0) {
        std::cerr << "Eroare la crearea procesului.\n";
    }
}

int main() {
    const int num_processes = 10;

    for (int i = 0; i < num_processes; ++i) {
        createChildProcess(i % 2); // Procesele sunt alternate între alb și negru
    }

    // Așteaptă finalizarea tuturor proceselor copil
    for (int i = 0; i < num_processes; ++i) {
        wait(nullptr);
    }

    return 0;
}
