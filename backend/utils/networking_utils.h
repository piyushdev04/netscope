#ifndef NETWORKING_UTILS_H
#define NETWORKING_UTILS_H

#include <string>
#include <regex>
#include <chrono>
#include <thread>
#include <iostream>

// Simple IP address validator
inline bool isValidIP(const std::string& ip) {
    const std::regex pattern(
        R"(^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}"
        R"(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$)");
    return std::regex_match(ip, pattern);
}

// Sleep for milliseconds
inline void sleepMs(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// Logger
inline void log(const std::string& msg) {
    std::cout << "[NetScope] " << msg << std::endl;
}

#endif // NETWORKING_UTILS_H