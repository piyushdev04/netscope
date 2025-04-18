#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define WHOIS_SERVER "whois.iana.org"
#define WHOIS_PORT 43

// Function to perform a WHOIS query over TCP
std::string whoisQuery(const std::string& domain) {
    int sockfd;
    struct sockaddr_in serverAddr;
    char buffer[4096];
    std::string response;

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return "Error: Unable to create socket";
    }

    // Set up the server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(WHOIS_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(WHOIS_SERVER);

    // Connect to the WHOIS server
    if (connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        return "Error: Unable to connect to WHOIS server";
    }

    // Send the WHOIS query for the domain
    std::string query = domain + "\r\n";
    send(sockfd, query.c_str(), query.size(), 0);

    // Receive the response from the WHOIS server
    ssize_t bytesReceived;
    while ((bytesReceived = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytesReceived] = '\0'; // Null terminate the received data
        response += buffer;
    }

    // Close the socket
    close(sockfd);

    return response;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <domain>" << std::endl;
        return 1;
    }

    const std::string domain = argv[1];

    std::cout << "Performing WHOIS lookup for: " << domain << std::endl;
    
    std::string result = whoisQuery(domain);

    std::cout << "WHOIS Response:\n" << result << std::endl;

    return 0;
}
