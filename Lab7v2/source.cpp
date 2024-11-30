#include <iostream>
#include <string>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <map>

std::mutex mtx;
std::condition_variable cv;
int current_type = -1; // Tipul curent (0 pentru alb, 1 pentru negru)
int active_processes = 0; // Numărul de fire active
std::queue<int> request_queue; // Coada cererilor
std::map<int, int> wait_counts; // Număr de cereri pentru fiecare tip
bool rotate_priority = false; // Flag pentru rotația priorității

void accessResource(int pid, int type) {
    std::unique_lock<std::mutex> lock(mtx);

    // Adaugă cererea în coadă și crește contorul pentru tipul respectiv
    request_queue.push(type);
    wait_counts[type]++;

    cv.wait(lock, [&] {
        bool same_type_access = (current_type == -1 || current_type == type);
        bool no_starvation = wait_counts[type] >= wait_counts[1 - type];
        return (!request_queue.empty() && request_queue.front() == type) &&
            same_type_access && no_starvation;
        });

    // Firele de același tip pot accesa resursa
    current_type = type;
    active_processes++;
    request_queue.pop();
    wait_counts[type]--;

    // Acces la resursă
    std::cout << "PID " << pid << " (tip " << (type == 0 ? "alb" : "negru")
        << ") acceseaza resursa.\n";

    lock.unlock();
    std::this_thread::sleep_for(std::chrono::seconds(1)); // Simulează utilizarea resursei
    lock.lock();

    // Eliberare resursă
    std::cout << "PID " << pid << " (tip " << (type == 0 ? "alb" : "negru")
        << ") elibereaza resursa.\n";
    active_processes--;

    // Dacă nu mai sunt fire active, resetați tipul curent și alternați prioritatea
    if (active_processes == 0) {
        current_type = -1;
        rotate_priority = !rotate_priority;
    }

    cv.notify_all();
}

void createChildProcess(int type) {
    pid_t pid = fork();

    if (pid == 0) {
        // Cod executat de procesul copil
        execl("./child_process", "child_process", std::to_string(type).c_str(), nullptr);
        // Dacă `execl` eșuează
        std::cerr << "Eroare la exec: " << strerror(errno) << "\n";
        _exit(1);
    }
    else if (pid > 0) {
        // Cod executat de procesul părinte
        int status;
        waitpid(pid, &status, 0); // Așteaptă finalizarea procesului copil
    }
    else {
        // Eroare la `fork`
        std::cerr << "Eroare la fork: " << strerror(errno) << "\n";
    }
}

int main() {
    const int num_processes = 10;

    std::vector<std::thread> threads;
    for (int i = 0; i < num_processes; ++i) {
        threads.emplace_back([i] {
            accessResource(getpid(), i % 2);
            });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}
