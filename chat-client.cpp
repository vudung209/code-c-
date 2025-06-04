#include <iostream>
#include <string>
#include <thread>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_PORT 12345
#define BUFFER_SIZE 1024

using namespace std;

void receiveMessages(int socketFd) {
    char buffer[BUFFER_SIZE + 1];
    while (true) {
        int bytesRead = recv(socketFd, buffer, BUFFER_SIZE, 0);
        if (bytesRead <= 0) {
            cout << "Disconnected from server." << endl;
            break;
        }
        buffer[bytesRead] = '\0';
        cout << buffer << endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <server_ip>" << endl;
        return 1;
    }

    string serverIP = argv[1];

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        cerr << "Socket creation failed." << endl;
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr) <= 0) {
        cerr << "Invalid address." << endl;
        close(sock);
        return 1;
    }

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Connection failed." << endl;
        close(sock);
        return 1;
    }

    cout << "Connected to server. Type messages and press Enter to send." << endl;

    thread receiver(receiveMessages, sock);

    string message;
    while (getline(cin, message)) {
        if (message.empty()) {
            continue;
        }
        if (send(sock, message.c_str(), message.size(), 0) < 0) {
            cerr << "Failed to send message." << endl;
            break;
        }
    }

    shutdown(sock, SHUT_RDWR);
    close(sock);
    receiver.join();
    return 0;
}
