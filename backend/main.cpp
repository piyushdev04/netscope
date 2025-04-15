#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <map>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8080

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) return "404 Not Found";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string getMimeType(const std::string& path) {
    if (path.size() >= 5 && path.substr(path.size() - 5) == ".html") return "text/html";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".css") return "text/css";
    if (path.size() >= 3 && path.substr(path.size() - 3) == ".js") return "application/javascript";
    if (path.size() >= 5 && path.substr(path.size() - 5) == ".json") return "application/json";
    return "text/plain";
}

// mock ping 
std::string handlePing() {
    srand(time(nullptr));
    int latency = rand() % 100 + 1;
    int ttl = 64;
    std::stringstream ss;
    ss << R"({"ip": "192.0.2.1", "latency_ms": )" << latency << R"(, "ttl": )" << ttl << "}";
    return ss.str();
}

// mock port scan
std::string handleScan() {
    return R"({"ip": "203.0.113.5", "open_ports": [22, 80, 443]})";
}

// mock Whois
std::string handleWhois() {
    return readFile("data/whois_data.json");
}

// mock IP info
std::string handleIpInfo() {
    return readFile("data/ip_info.json");
}

void handleClient(int clientSocket) {
    char buffer[4096];
    read(clientSocket, buffer, 4096);
    std::string request(buffer);

    std::string responseBody, contentType;
    int statusCode = 200;

    if (request.find("GET /api/ping") == 0) {
        responseBody = handlePing();
        contentType = "application/json";
    } else if (request.find("GET /api/scan") == 0) {
        responseBody = handleScan();
        contentType = "application/json";
    } else if (request.find("GET /api/whois") == 0) {
        responseBody = handleWhois();
        contentType = "application/json";
    } else if (request.find("GET /api/ip") == 0) {
        responseBody = handleIpInfo();
        contentType = "application/json";
    } else {
        std::string filePath = "public/index.html";
        std::string urlPath = request.substr(4, request.find(" ", 4) - 4);
        if (urlPath != "/") {
            filePath = "public" + urlPath;
        }

        if (std::filesystem::exists(filePath)) {
            responseBody = readFile(filePath);
            contentType = getMimeType(filePath);
        } else {
            responseBody = "404 Not Found";
            statusCode = 404;
            contentType = "text/plain";
        }
    }

    std::stringstream response;
    response << "HTTP/1.1 " << statusCode << " OK\r\n";
    response << "Content-Type: " << contentType << "\r\n";
    response << "Content-Length: " << responseBody.size() << "\r\n";
    response << "Connection: close\r\n\r\n";
    response << responseBody;

    send(clientSocket, response.str().c_str(), response.str().size(), 0);
    close(clientSocket);
}

int main() {
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(serverFd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(serverFd, 10) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "[NetScope] Server running on http://localhost:" << PORT << std::endl;

    while (true) {
        int clientSocket = accept(serverFd, nullptr, nullptr);
        std::thread(handleClient, clientSocket).detach();
    }

    return 0;
}
