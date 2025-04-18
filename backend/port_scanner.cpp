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

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <hostname or IP> <start_port-end_port>" << std::endl;
        return 1;
    }

    const std::string target_ip = argv[1];
    std::string range = argv[2];
    int start_port, end_port;

    // Parse the port range (e.g., 20-1000)
    size_t dash_pos = range.find('-');
    if (dash_pos == std::string::npos) {
        std::cerr << "Invalid port range format. Use <start_port-end_port>" << std::endl;
        return 1;
    }

    start_port = std::stoi(range.substr(0, dash_pos));
    end_port = std::stoi(range.substr(dash_pos + 1));

    std::cout << "Scanning ports on " << target_ip << " from " << start_port << " to " << end_port << std::endl;

    // Scan the ports in the specified range
    for (int port = start_port; port <= end_port; ++port) {
        if (isPortOpen(target_ip, port)) {
            std::cout << "Port " << port << " is open." << std::endl;
        } else {
            std::cout << "Port " << port << " is closed." << std::endl;
        }
    }

    return 0;
}