#pragma once

#include <exception>
#include <string>

class scanner_exception : public std::exception {
private:
    std::string message;

public:
    explicit scanner_exception(std::string &&msg) : message(msg) {}

    explicit scanner_exception(std::string &msg) : message(msg) {}

    [[nodiscard]] const char *what() const noexcept override {
        return message.c_str();
    }
};
