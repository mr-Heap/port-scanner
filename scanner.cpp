#include "scanner.h"



std::vector<int> scanner::scan_ports(int start_port, int end_port, int num_threads) {
    check_IP(ip_);
    if (start_port > end_port || start_port < MIN_PORT || end_port > MAX_PORT) {
        throw scanner_exception("Invalid ports range: [" + std::to_string(start_port) + ";" + std::to_string(end_port) + "]");
    }
    std::vector<std::thread> threads;
    int amount = end_port - start_port + 1;
    threads.reserve(amount);
    std::vector<int> open_ports;
    std::exception_ptr ex = nullptr;
    int threads_count = std::max(std::min(num_threads, amount), 1);
    int module = amount % threads_count;
    int segment = amount / threads_count;
    int position = 1;
    auto runnable = [&](int start, int end) {
        try {
            scan_ports0(start, end, open_ports, threads_count > 1);
        } catch (std::runtime_error & error) {
            ex = std::current_exception(); // catch one
        }
    };

    for (int i = 0; i < threads_count; ++i) {
        int offset = segment + (i < module);
        int start = position;
        int end = position + offset - 1;
        threads.emplace_back(runnable, start, end);
        position += offset;
    }
    for (std::thread & thread : threads) {
        thread.join();
    }
    if (ex != nullptr) {
        std::rethrow_exception(ex);
    }
    return open_ports;
}

bool scanner::scan_port(int port) {
    epoll epoll = make_epoll(ip_);
    return scan_port0(epoll, port);
}

bool scanner::scan_port0(epoll &epoll, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error creating socket" << std::endl;
        throw std::runtime_error("Socket exception");
    }

    fcntl(sockfd, F_SETFL, O_NONBLOCK);
    epoll.target_.sin_port = htons(port);
    int code = connect(sockfd, (struct sockaddr *) &epoll.target_, sizeof(epoll.target_));
    if (code == -1 && errno != EINPROGRESS) {
        close(sockfd);
        std::string msg = "Connection failed " + std::string(strerror(errno));
        std::cerr << msg << std::endl;
        throw std::runtime_error(msg);
    }

    epoll.event_.events = EPOLLOUT;
    epoll.event_.data.fd = sockfd;

    if (epoll_ctl(epoll.epoll_fd_, EPOLL_CTL_ADD, sockfd, &epoll.event_) == -1) {
        std::cerr << "Error adding socket to epoll" << std::endl;
        close(sockfd);
        throw std::runtime_error("Error adding socket to epoll");
    }

    int nfds = epoll_wait(epoll.epoll_fd_, epoll.events_, 1, 1000);  // 1 second timeout
    bool is_open = false;
    if (nfds > 0) {
        int so_error;
        socklen_t len = sizeof so_error;
        getsockopt(epoll.events_[0].data.fd, SOL_SOCKET, SO_ERROR, &so_error, &len);
        if (so_error == 0) {
            is_open = true;
        }
    }

    close(sockfd);
    return is_open;
}


void scanner::scan_ports0(int start_port, int end_port, std::vector<int> &open_ports, bool parallel) {
    epoll epoll = make_epoll(ip_);
    for (int port = start_port; port <= end_port; port++) {
        if (scan_port0(epoll, port)) {
            if (parallel) {
                std::lock_guard<std::mutex> lock(mutex_);
                open_ports.emplace_back(port);
            } else {
                open_ports.emplace_back(port);
            }
        }
    }
}


scanner::epoll scanner::make_epoll(const char *ip) {
    int fd = epoll_create1(0);
    if (fd == -1) {
        throw scanner_exception("Error creating epoll instance");
    }

    return {fd, ip};
}


void scanner::check_IP(const char *ip) {
    std::stringstream ss(ip);
    std::string token;
    char delimiter = '.';
    int cnt = 0;
    while (std::getline(ss, token, delimiter)) {
        if (token.empty() || !std::all_of(token.begin(), token.end(), ::isdigit)) {
            throw scanner_exception("Bad token in ip-address on position " + std::to_string(cnt));
        }
        int number = std::stoi(token);
        if (number < 0 || number > 255) {
            throw scanner_exception("Invalid IP token");
        }
        ++cnt;
    }
    if (cnt != 4) {
        throw scanner_exception("Invalid IP-address");
    }
}