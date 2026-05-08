#include <iostream>
#include <string>

int runClient(const std::string& ip = "127.0.0.1");
void startServerThread();

void showMenu() {
    std::cout << "=== Multiplayer Project ===\n";
    std::cout << "1. Host Game\n";
    std::cout << "2. Join Game\n";
    std::cout << "3. Quit\n";
    std::cout << "> ";

    int choice;
    std::cin >> choice;

    if (choice == 1) {
        startServerThread();
        runClient("127.0.0.1");
    }
    else if (choice == 2) {
        std::string ip;
        std::cout << "Enter IP: ";
        std::cin >> ip;
        runClient(ip);
    }
}

int main(int argc, char** argv) {

    // Command-line mode
    if (argc >= 2) {
        std::string arg = argv[1];

        if (arg == "--host") {
            startServerThread();
            return runClient("127.0.0.1");
        }

        if (arg == "--join" && argc >= 3) {
            return runClient(argv[2]);
        }

        std::cout << "Unknown argument\n";
        return 0;
    }

    // Menu mode
    showMenu();
    return 0;
}
