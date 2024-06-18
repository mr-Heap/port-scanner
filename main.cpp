#include <iostream>
#include "scanner.h"

int main(int argc, char *argv[]) { // TODO: make from command line
    if (argc != 4) {
        std::cerr << "Usage: <ip> <startPort> <endPort>" << std::endl;
        return 1;
    }
    try {
        int start_port = std::stoi(argv[2]);
        int end_port = std::stoi(argv[3]);
        scanner scanner(argv[1]);
        int threads = static_cast<int>(std::thread::hardware_concurrency()); // C++11 and higher
        std::vector<int> open_ports = scanner.scan_ports(start_port, end_port, threads);
        for (int & port : open_ports) {
            std::cout << "Port " << port << " is open" << "\n";
        }
    } catch (scanner_exception & ex) {
        std::cerr << "Scanner exception: " << ex.what() << std::endl;
        return 1;
    } catch (std::invalid_argument& ex) {
        std::cerr << "Number of port in not a number" << std::endl;
        return 1;
    } catch (std::runtime_error & error) {
        std::cerr << "Runtime error " << error.what() << std::endl;
        return 1;
    } catch (std::exception & ex) {
        std::cerr << "Undefined exception " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
