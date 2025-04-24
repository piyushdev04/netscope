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
#include "handlers.h"

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

    std::cout << "Requested path: " << path << std::endl;  // Debugging line

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
        std::string filePath = "../public/index.html";
        if (path != "/") filePath = "../public" + path;

        std::cout << "Attempting to serve file: " << filePath << std::endl;  // Debugging line

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
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server is listening on port " << PORT << "..." << std::endl;

    while ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) >= 0) {
        std::thread(handleClient, new_socket).detach();
    }

    return 0;
}