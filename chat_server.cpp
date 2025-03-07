#include<iostream>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<pthread.h>

using namespace std;

#define MESSAGE_LENGTH 1024 // Максимальный размер буфера для данных
#define PORT 7777 // Порт для связи

struct sockaddr_in serveraddress, client;
socklen_t length;
int sockert_file_descriptor, connection, bind_status, connection_status;
char message[MESSAGE_LENGTH];

void* client_handler(void* client_socket) {
    int client_sock = *(int*)client_socket;
    while (1) {
        bzero(message, MESSAGE_LENGTH);
        ssize_t bytes = read(client_sock, message, sizeof(message));
        if (bytes > 0) {
            if (strncmp(message, "end", 3) == 0) {
                cout << "Client Disconnected." << endl;
                break;
            }
            cout << "Data received from client: " << message << endl;

            bzero(message, MESSAGE_LENGTH);
        }
    }
    close(client_sock);
    return nullptr;
}

int main() {
    // Создадим сокет
    sockert_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (sockert_file_descriptor == -1) {
        cout << "Socket creation failed!" << endl;
        exit(1);
    }

    serveraddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddress.sin_port = htons(PORT);
    serveraddress.sin_family = AF_INET;

    bind_status = bind(sockert_file_descriptor, (struct sockaddr*)&serveraddress, sizeof(serveraddress));
    if (bind_status == -1) {
        cout << "Socket binding failed!" << endl;
        exit(1);
    }

    connection_status = listen(sockert_file_descriptor, 5);
    if (connection_status == -1) {
        cout << "Socket is unable to listen for new connections!" << endl;
        exit(1);
    } else {
        cout << "Server is listening for new connections..." << endl;
    }

    length = sizeof(client);
    while (1) {
        connection = accept(sockert_file_descriptor, (struct sockaddr*)&client, &length);
        if (connection == -1) {
            cout << "Server is unable to accept the data from client!" << endl;
            exit(1);
        }

        pthread_t client_thread;
        pthread_create(&client_thread, nullptr, client_handler, (void*)&connection);
    }

    close(sockert_file_descriptor);
    return 0;
}
