//
// Created by marina on 11.06.24.
//
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <sys/epoll.h>
#include <fcntl.h>
#include "exception.h"


#define MIN_PORT 1
#define MAX_PORT 65535

class scanner {

private:
    struct epoll {
        epoll(int fd, const char *ip) : epoll_fd_(fd) {
            target_.sin_family = AF_INET;
            target_.sin_addr.s_addr = inet_addr(ip);
        }

        int epoll_fd_;
        epoll_event event_{};
        epoll_event events_[1]{};
        sockaddr_in target_{};
    };



public:
    explicit scanner(const char *ip) : ip_(ip) {}

    /*
     * Returns list of open ports from {start_port} to {end_port}
     */
    std::vector<int> scan_ports(int start_port, int end_port, int num_threads = 1);

    bool scan_port(int port);

private:
    void check_IP(const char *ip);

    void scan_ports0(int start_port, int end_port, std::vector<int> & open_ports, bool parallel = false);

    bool scan_port0(epoll & epoll, int port);

    epoll make_epoll(const char * ip);

    const char *ip_;
    std::mutex mutex_{};
};


