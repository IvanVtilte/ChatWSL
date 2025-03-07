#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unordered_map>
#include "user.h"

using namespace std;

#define MESSAGE_LENGTH 1024 // Максимальный размер буфера для данных
#define PORT 7777 // Порт для связи

std::unordered_map<std::string, User*> users;

int socket_file_descriptor, connection;
struct sockaddr_in serveraddress;
char message[MESSAGE_LENGTH];

void* receive_messages(void* socket) {
    int sock = *(int*)socket;
    while (1) {
        bzero(message, sizeof(message));
        ssize_t bytes = read(sock, message, sizeof(message));
        if (bytes > 0) {
            // При получении сообщения выводим имя отправителя и само сообщение
            cout << "Сообщение от " << message << endl;  // Выводим имя пользователя и сообщение
        }
    }
    return nullptr;    
}

void registerUser(const std::string& username, const std::string& password, const std::string& name) {
    if (users.find(username) != users.end()) {
        throw std::runtime_error("Пользователь с таким логином уже существует!");
    }
    users[username] = new User(username, password, name);
    std::cout << "Пользователь " << name << " зарегистрирован." << std::endl;
}

User* authenticateUser(const std::string& username, const std::string& password) {
    auto it = users.find(username);
    if (it == users.end()) {
        throw std::runtime_error("Пользователь не найден!");
    }
    User* user = it->second;
    if (user->password != password) {
        throw std::runtime_error("Неверный пароль!");
    }
    return user;
}

void showMenu() {
    cout << "\n1. Регистрация нового пользователя" << endl;
    cout << "2. Вход в систему" << endl;
    cout << "3. Отправить сообщение на сервер" << endl;
    cout << "0. Выход" << endl;
    cout << "Выберите опцию: ";
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "Использование: ./client <server_address>" << endl;
        return 1;
    }

    // Получаем IP адрес сервера из аргумента командной строки
    const char* server_ip = argv[1];

    socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_file_descriptor == -1) {
        cout << "Socket creation failed!" << endl;
        exit(1);
    }

    serveraddress.sin_addr.s_addr = inet_addr(server_ip);
    serveraddress.sin_port = htons(PORT);
    serveraddress.sin_family = AF_INET;

    connection = connect(socket_file_descriptor, (struct sockaddr*)&serveraddress, sizeof(serveraddress));
    if (connection == -1) {
        cout << "Connection with the server failed!" << endl;
        exit(1);
    }

    // Создаем поток для получения сообщений от сервера
    pthread_t receive_thread;
    pthread_create(&receive_thread, nullptr, receive_messages, (void*)&socket_file_descriptor);

    bool is_logged_in = false;
    User* current_user = nullptr;

    while (1) {
        showMenu();
        int choice;
        cin >> choice;

        switch (choice) {
            case 1: { // Регистрация
                string username, password, name;
                cout << "Введите имя пользователя: ";
                cin >> username;
                cout << "Введите пароль: ";
                cin >> password;
                cout << "Введите ваше имя: ";
                cin >> name;

                try {
                    registerUser(username, password, name);
                } catch (const std::exception& e) {
                    cout << e.what() << endl;
                }
                break;
            }
            case 2: { // Вход в систему
                string username, password;
                cout << "Введите имя пользователя: ";
                cin >> username;
                cout << "Введите пароль: ";
                cin >> password;

                try {
                    current_user = authenticateUser(username, password);
                    is_logged_in = true;
                    cout << "Добро пожаловать, " << current_user->name << "!" << endl;
                } catch (const std::exception& e) {
                    cout << e.what() << endl;
                }
                break;
            }
            case 3: { // Отправить сообщение на сервер
                if (!is_logged_in) {
                    cout << "Пожалуйста, войдите в систему перед отправкой сообщений!" << endl;
                    break;
                }

                bzero(message, sizeof(message));
                cout << "Введите сообщение для отправки на сервер: ";
                cin.ignore(); // Чтобы очистить буфер перед вводом строки
                cin.getline(message, MESSAGE_LENGTH);

                if (strncmp(message, "end", 3) == 0) {
                    write(socket_file_descriptor, message, sizeof(message));
                    cout << "Client Exit." << endl;
                    close(socket_file_descriptor);
                    return 0;
                }

                // Добавляем имя пользователя в начало сообщения
                string full_message = current_user->username + ": " + message;
                write(socket_file_descriptor, full_message.c_str(), full_message.length());
                cout << "Сообщение отправлено на сервер!" << endl;
                break;
            }
            case 0: { // Выход
                cout << "Выход из программы." << endl;
                close(socket_file_descriptor);
                return 0;
            }
            default:
                cout << "Неверный выбор! Пожалуйста, выберите правильную опцию." << endl;
        }
    }

    close(socket_file_descriptor);
    return 0;
}
