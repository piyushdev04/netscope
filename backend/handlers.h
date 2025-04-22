#ifndef HANDLERS_H
#define HANDLERS_H

#include <string>

// Function declarations for the handlers
std::string handlePing(const std::string& host);
std::string handleScan(const std::string& ip);
std::string handleWhois(const std::string& domain);
std::string handleIpInfo();

#endif