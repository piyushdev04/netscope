#include <iostream>
#include <thread>
#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <filesystem>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

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


// Forward declarations of tool handlers
std::string handlePing(const std::string& host);
std::string handleScan(const std::string& ip);
std::string handleWhois(const std::string& domain);
std::string handleIpInfo();

std::map<std::string, std::string> parseQueryParams(const std::string& request) {
    std::map<std::string, std::string> params;
    auto qPos = request.find('?');
    if (qPos == std::string::npos) return params;

    auto query = request.substr(qPos + 1);
    std::stringstream ss(query);
    std::string token;
    while (getline(ss, token, '&')) {
        auto eqPos = token.find('=');
        if (eqPos != std::string::npos) {
            auto key = token.substr(0, eqPos);
            auto val = token.substr(eqPos + 1);
            params[key] = val;
        }
    }
    return params;
}

void handleClient(int clientSocket) {
    char buffer[8192] = {0};
    read(clientSocket, buffer, sizeof(buffer));
    std::string request(buffer);

    std::string path = request.substr(4, request.find(' ', 4) - 4);
    std::string responseBody;
    std::string contentType = "text/plain";
    int statusCode = 200;

    if (path.rfind("/api/ping", 0) == 0) {
        auto params = parseQueryParams(path);
        std::string host = params["host"];
        responseBody = handlePing(host);
        contentType = "application/json";
    } else if (path.rfind("/api/scan", 0) == 0) {
        auto params = parseQueryParams(path);
        std::string ip = params["ip"];
        responseBody = handleScan(ip);
        contentType = "application/json";
    } else if (path.rfind("/api/whois", 0) == 0) {
        auto params = parseQueryParams(path);
        std::string domain = params["domain"];
        responseBody = handleWhois(domain);
        contentType = "application/json";
    } else if (path == "/api/ipinfo") {
        responseBody = handleIpInfo();
        contentType = "application/json";
    } else {
        // Static file handling
        std::string filePath = "public/index.html";
        if (path != "/") filePath = "public" + path;
        if (std::filesystem::exists(filePath)) {
            responseBody = readFile(filePath);
            contentType = getMimeType(filePath);
        } else {
            responseBody = "404 Not Found";
            statusCode = 404;
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
    if (serverFd == -1) {
        perror("Socket creation failed");
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverFd, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    if (listen(serverFd, 10) < 0) {
        perror("Listen failed");
        return 1;
    }

    std::cout << "[NetScope] Server started on http://localhost:" << PORT << "\n";

    while (true) {
        int clientSocket = accept(serverFd, nullptr, nullptr);
        std::thread(handleClient, clientSocket).detach();
    }

    return 0;
}
