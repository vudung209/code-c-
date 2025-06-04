#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <cstring>

#define SERVER_PORT 12345
#define BUFFER_SIZE 1024

using namespace std;

vector<int> clients;
mutex clientsMutex;

void broadcastMessage(const string& message, int sender) {
    lock_guard<mutex> lock(clientsMutex);
    for (int client : clients) {
        if (client != sender) {
            send(client, message.c_str(), message.length(), 0);
        }
    }
}

void handleClient(int clientSocket) {
    char buffer[BUFFER_SIZE + 1];
    while (true) {
        int recvResult = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (recvResult <= 0) {
            cout << "Client disconnected!" << endl;
            close(clientSocket);
            lock_guard<mutex> lock(clientsMutex);
            clients.erase(remove(clients.begin(), clients.end(), clientSocket), clients.end());
            break;
        }
        buffer[recvResult] = '\0';
        cout << "Received: " << buffer << endl;
        broadcastMessage(buffer, clientSocket);
    }
}

void startServer() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 5);

    cout << "Server started. Waiting for connections..." << endl;

    while (true) {
        sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &addrLen);
        if (clientSocket != -1) {
            {
                lock_guard<mutex> lock(clientsMutex);
                clients.push_back(clientSocket);
            }
            cout << "Client connected!" << endl;
            thread t(handleClient, clientSocket);
            t.detach();
        }
    }

    close(serverSocket);
}

int main() {
    startServer();
    return 0;
}
