#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "json.hpp"
#include "screenshot.hpp" // 包含截图功能的头文件

#pragma comment(lib, "ws2_32.lib")

using json = nlohmann::json;

// 读取配置文件
std::pair<std::string, int> ReadConfig() {
    try {
        std::ifstream configFile("config.json");
        if (!configFile.is_open()) {
            std::cerr << "Failed to open config.json" << std::endl;
            exit(EXIT_FAILURE);
        }

        json config;
        configFile >> config;

        // 检查字段是否存在并且类型是否正确
        if (!config.contains("server_ip") || !config["server_ip"].is_string()) {
            std::cerr << "Invalid or missing 'server_ip' in config.json" << std::endl;
            exit(EXIT_FAILURE);
        }

        if (!config.contains("server_port") || !config["server_port"].is_number_integer()) {
            std::cerr << "Invalid or missing 'server_port' in config.json" << std::endl;
            exit(EXIT_FAILURE);
        }

        std::string serverIp = config.at("server_ip").get<std::string>();
        int serverPort = config.at("server_port").get<int>();

        return std::make_pair(serverIp, serverPort);
    }
    catch (const json::exception& e) {
        std::cerr << "JSON exception: " << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}

// 连接到服务器
SOCKET ConnectToServer(const std::string& ip, int port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed" << std::endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr.s_addr);
    serverAddr.sin_port = htons(port);

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to server" << std::endl;
        closesocket(sock);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    return sock;
}

// 监听命令
void ListenForCommands(SOCKET sock) {
    char buffer[1024];
    while (true) {
        int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            std::string command(buffer, bytesReceived);
            if (command == "SCREENSHOT") {
                CaptureScreenshot(); // 直接调用截图函数

                // 读取并发送 screenshot.bmp 文件
                std::ifstream screenshot("screenshot.bmp", std::ios::binary);
                if (!screenshot) {
                    std::cerr << "Failed to open screenshot.bmp" << std::endl;
                    continue;
                }

                screenshot.seekg(0, std::ios::end);
                std::streamsize size = screenshot.tellg();
                screenshot.seekg(0, std::ios::beg);

                std::vector<char> fileBuffer(size);
                if (screenshot.read(fileBuffer.data(), size)) {
                    send(sock, fileBuffer.data(), size, 0);
                }
                else {
                    std::cerr << "Failed to read screenshot.bmp" << std::endl;
                }
            }
        }
        else if (bytesReceived == 0) {
            // Connection closed
            break;
        }
        else {
            std::cerr << "recv failed with error: " << WSAGetLastError() << std::endl;
            break;
        }
    }
}

int main() {
    // 读取配置
    auto config = ReadConfig();
    std::string serverIp = config.first;
    int serverPort = config.second;

    // 连接服务器
    SOCKET sock = ConnectToServer(serverIp, serverPort);

    // 开始监听命令
    ListenForCommands(sock);

    // 关闭套接字
    closesocket(sock);
    WSACleanup();

    return 0;
}
