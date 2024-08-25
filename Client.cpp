#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "json.hpp"
#include "screenshot.hpp" // ������ͼ���ܵ�ͷ�ļ�

#pragma comment(lib, "ws2_32.lib")

using json = nlohmann::json;

// ��ȡ�����ļ�
std::pair<std::string, int> ReadConfig() {
    std::ifstream configFile("config.json");
    if (!configFile.is_open()) {
        std::cerr << "Failed to open config.json" << std::endl;
        exit(EXIT_FAILURE);
    }

    json config;
    configFile >> config;

    std::string serverIp = config["server_ip"];
    int serverPort = config["server_port"];
    return std::make_pair(serverIp, serverPort);
}

// ���ӵ�������
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

// ��������
void ListenForCommands(SOCKET sock) {
    char buffer[1024];
    while (true) {
        int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            std::string command(buffer, bytesReceived);
            if (command == "SCREENSHOT") {
                CaptureScreenshot(true);

                // ��ȡ������screenshot.bmp�ļ�
                std::ifstream screenshot("screenshot.bmp", std::ios::binary);
                screenshot.seekg(0, std::ios::end);
                std::streamsize size = screenshot.tellg();
                screenshot.seekg(0, std::ios::beg);

                std::vector<char> fileBuffer(size);
                if (screenshot.read(fileBuffer.data(), size)) {
                    send(sock, fileBuffer.data(), static_cast<int>(size), 0);
                }
            }
        }
    }
}

int main() {
    // ��ȡ����
    auto config = ReadConfig();
    std::string serverIp = config.first;
    int serverPort = config.second;

    // ���ӷ�����
    SOCKET sock = ConnectToServer(serverIp, serverPort);

    // ��ʼ��������
    ListenForCommands(sock);

    // �ر��׽���
    closesocket(sock);
    WSACleanup();

    return 0;
}
