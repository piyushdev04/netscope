#include "handlers.h"
#include "ping_tool.h"       // Include the pingTool function declaration
#include "port_scanner.h"    // Include the portScanner function declaration
#include "whois_lookup.h"    // Include the whois lookup function
#include "ip_info.h"         // Include the IP info function
#include <string>

// Function definitions for the handlers
std::string handlePing(const std::string& host) {
    return pingTool(host);  // Calling the pingTool function defined in ping_tool.cpp
}

std::string handleScan(const std::string& ip) {
    int startPort = 1; // You can define your port range
    int endPort = 1024;
    
    return portScanner(ip, startPort, endPort); // Now passing the correct parameters
}

std::string handleWhois(const std::string& domain) {
    return whoisQuery(domain);  // Calling the Whois function defined in whois_lookup.cpp
}

std::string handleIpInfo() {
    return getPublicIP("https://api.ipify.org");  // Calling the IP info function defined in ip_info.cpp
}