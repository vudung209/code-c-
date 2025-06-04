#include <iostream>
#include <winsock2.h>
#include <thread>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 12345
#define BUFFER_SIZE 1024

using namespace std;

vector<SOCKET> clients;

void broadcastMessage(const string& message, SOCKET sender) {
    for (SOCKET client : clients) {
        if (client != sender) {
            send(client, message.c_str(), message.length(), 0);
        }
    }
}

void handleClient(SOCKET clientSocket) {
    char buffer[BUFFER_SIZE];
    while (true) {
        int recvResult = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (recvResult > 0) {
            buffer[recvResult] = '\0';
            cout << "Received message: " << buffer << endl;
            broadcastMessage(buffer, clientSocket);
        } else {
            cout << "Client disconnected!" << endl;
            closesocket(clientSocket);
            break;
        }
    }
}

void startServer() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 5);

    cout << "Server started. Waiting for connections..." << endl;

    while (true) {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket != INVALID_SOCKET) {
            clients.push_back(clientSocket);
            cout << "Client connected!" << endl;
            thread clientThread(handleClient, clientSocket);
            clientThread.detach();
        }
    }

    WSACleanup();
}

int main() {
    startServer();
    return 0;
}
