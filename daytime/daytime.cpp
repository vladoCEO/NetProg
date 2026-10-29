
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int rc;

    // 1. Создаём и заполняем адрес сервера
    sockaddr_in *srv_addr = new sockaddr_in{};
    srv_addr->sin_family = AF_INET;
    srv_addr->sin_port = htons(13);
    srv_addr->sin_addr.s_addr = inet_addr("172.16.40.1");

    // 2. Создаём UDP-сокет
    int s = socket(AF_INET, SOCK_DGRAM, 0);

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

    // 5. Задаём адрес удалённого узла для UDP-сокета
    rc = connect(s, (const sockaddr*)srv_addr,
                 sizeof(sockaddr_in));

    if (rc == -1) {
        std::cerr << "Ошибка connect()\n";
        close(s);
        delete self_addr;
        delete srv_addr;
        return 1;
    }

    // 6. Отправляем пустой запрос службе Daytime
    rc = send(s, "", 0, 0);

    if (rc == -1) {
        std::cerr << "Ошибка send()\n";
        close(s);
        delete self_addr;
        delete srv_addr;
        return 1;
    }

    // 7. Получаем ответ сервера
    char buffer[257];

    rc = recv(s, buffer, sizeof(buffer) - 1, 0);

    if (rc == -1) {
        std::cerr << "Ошибка recv()\n";
        close(s);
        delete self_addr;
        delete srv_addr;
        return 1;
    }

    buffer[rc] = '\0';

    // 8. Выводим полученные данные
    std::cout << "Ответ сервера Daytime:\n";
    std::cout << buffer << '\n';

    // 9. Закрываем сокет и освобождаем память
    close(s);
    delete self_addr;
    delete srv_addr;

    return 0;
}
