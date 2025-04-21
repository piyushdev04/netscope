#include "port_scanner.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

#define TIMEOUT 1 // 1 second timeout

// Function to check if a port is open
bool isPortOpen(const std::string& ip, int port) {
    int sockfd;
    struct sockaddr_in serverAddr;

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return false;
    }

    // Set up the server address struct
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());

    // Set the socket to non-blocking
    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    // Try to connect to the target IP and port
    int result = connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    // Check if the connection was successful (0 means success)
    if (result == 0) {
        close(sockfd);
        return true; // Port is open
    }

    // Otherwise, the connection failed (timeout or refusal)
    close(sockfd);
    return false;
}

// Function to scan a range of ports on a given IP
std::string portScanner(const std::string& ip, int startPort, int endPort) {
    std::string result = "Scanning ports on " + ip + " from " + std::to_string(startPort) + " to " + std::to_string(endPort) + ":\n";
    
    for (int port = startPort; port <= endPort; ++port) {
        if (isPortOpen(ip, port)) {
            result += "Port " + std::to_string(port) + " is open.\n";
        } else {
            result += "Port " + std::to_string(port) + " is closed.\n";
        }
    }
    return result;
}