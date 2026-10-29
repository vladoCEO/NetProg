
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int rc;

    // 1. Создаём и заполняем адрес Echo-сервера
    sockaddr_in *srv_addr = new sockaddr_in{};
    srv_addr->sin_family = AF_INET;
    srv_addr->sin_port = htons(7);
    srv_addr->sin_addr.s_addr = inet_addr("172.16.40.13");

    // 2. Создаём TCP-сокет
    int s = socket(AF_INET, SOCK_STREAM, 0);

    if (s == -1) {
        std::cerr << "Ошибка socket()\n";
        delete srv_addr;
        return 1;
    }

    // 3. Создаём и заполняем локальный адрес
    sockaddr_in *self_addr = new sockaddr_in{};
    self_addr->sin_family = AF_INET;
    self_addr->sin_port = htons(0);
    self_addr->sin_addr.s_addr = htonl(INADDR_ANY);

    // 4. Привязываем сокет к локальному адресу
    rc = bind(s, (const sockaddr*)self_addr,
              sizeof(sockaddr_in));

    if (rc == -1) {
        std::cerr << "Ошибка bind()\n";
        close(s);
        delete self_addr;
        delete srv_addr;
        return 1;
    }

    // 5. Устанавливаем TCP-соединение с сервером
    rc = connect(s, (const sockaddr*)srv_addr,
                 sizeof(sockaddr_in));

    if (rc == -1) {
        std::cerr << "Ошибка connect(): сервер недоступен\n";
        close(s);
        delete self_addr;
        delete srv_addr;
        return 1;
    }

    // 6. Вводим сообщение для отправки
    std::string message;

    std::cout << "Введите сообщение: ";
    std::getline(std::cin, message);

    if (message.empty()) {
        std::cerr << "Сообщение пустое\n";
        close(s);
        delete self_addr;
        delete srv_addr;
        return 1;
    }

    // 7. Отправляем сообщение серверу
    size_t total_sent = 0;

    while (total_sent < message.size()) {
        rc = send(s,
                  message.data() + total_sent,
                  message.size() - total_sent,
                  0);

        if (rc <= 0) {
            std::cerr << "Ошибка send()\n";
            close(s);
            delete self_addr;
            delete srv_addr;
            return 1;
        }

        total_sent += rc;
    }

    // 8. Получаем эхо-ответ сервера
    std::string response(message.size(), '\0');
    size_t total_received = 0;

    while (total_received < response.size()) {
        rc = recv(s,
                  &response[total_received],
                  response.size() - total_received,
                  0);

        if (rc <= 0) {
            std::cerr << "Ошибка recv(): ответ не получен\n";
            close(s);
            delete self_addr;
            delete srv_addr;
            return 1;
        }

        total_received += rc;
    }

    // 9. Выводим ответ
    std::cout << "Ответ сервера: " << response << '\n';

    // 10. Закрываем сокет и освобождаем память
    close(s);
    delete self_addr;
    delete srv_addr;

    return 0;
}
