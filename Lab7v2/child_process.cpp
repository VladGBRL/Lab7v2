#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <unistd.h>

void accessResource(pid_t pid, int type) {
    std::cout << "PID " << pid << " (tip " << (type == 0 ? "alb" : "negru")
        << ") acceseaza resursa.\n";

    std::this_thread::sleep_for(std::chrono::seconds(1)); // Simulează utilizarea resursei

    std::cout << "PID " << pid << " (tip " << (type == 0 ? "alb" : "negru")
        << ") elibereaza resursa.\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Argumente insuficiente!\n";
        return 1;
    }

    int type = std::stoi(argv[1]); // Tipul procesului (0 pentru alb, 1 pentru negru)

    accessResource(getpid(), type);

    return 0;
}
