// Это реализация сервера. Для него необходимо:
// - создать файловый дескриптор сокета
// - закрепить сетевой адрес за сокетом
// - начать прослушивание сокета
// - принять соединение
// - читать/писать данные по сокету принятного соединения

#include <network_common.hpp>
#include "serialized_data.hpp"

#include <iostream>
#include <cstring>

int main()
{
    Startup();

    // Создаем файловый дескриптор сокета:
    // - AF_INET означает "Address Family - internet IPv4"
    // - SOCK_STREAM - это потоковый сокет, для взаимодействия по сети будет использоваться
    //                 протокол транспортного уровня TCP (выполняется контроль передачи)
    // - 0 - протокол выбирается автоматически, однако можно указать что-либо конкретное
    socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        std::cerr << "Error creating socket: " << std::strerror( errno ) << std::endl;
        return 1;
    }

    // Задаем адрес сервера, по которому будет закреплен сокет 
    // Например, 127.0.0.1:12345
    // По хорошему, этот адрес должен быть не петлей, но для теста сойдет
    std::string address = "127.0.0.1";
    short port = 12345;

    // Этими действиями мы преобразуем адрес к нужному внутреннему представлению
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    // Обратите внимание на функцию htons - она расшифровывается как
    // Host TO Network Short - перевести порядок следования байт в числе типа short из хостового в сетевой
    // Сетевой порядок следования байт - это Big Endian, а на хосте с архитектурой x86 - Little Endian.
    addr.sin_port = htons(port);
    inet_pton(AF_INET, address.c_str(), &addr.sin_addr);

    // Закрепляем сокет сервера за переданным адресом
    int rv = bind(sock, reinterpret_cast< sockaddr* >( &addr ), sizeof(sockaddr_in));
    if (rv == -1)
    {
        std::cerr << "Error binding socket: " << std::strerror( errno ) << std::endl;
        close(sock); // При любой ошибке и дальше нужно сначала закрыть сокет, а потом выйти
        return 1;
    }

    // Прослушиваем сокет на наличие ожидающих обработки подключений
    // Второй аргумент здесь - это максимальное количество ожидаемых подключений в очереди
    rv = listen(sock, 1);
    if (rv == -1)
    {
        std::cerr << "Error listening socket: " << std::strerror( errno ) << std::endl;
        close(sock);
        return 1;
    }

    std::cout << "Started listening connections on " << address << ":" << port << std::endl;

    // Принимаем подключение, если оно есть, а если его нет - просто блокируемся и ждем
    // Два следующих аргумента нужны для сохранения адреса подключившегося клиента, мы их не используем
    socket_t client = accept(sock, nullptr, nullptr);
    if (client == INVALID_SOCKET)
    {
        std::cerr << "Error accepting client: " << std::strerror( errno ) << std::endl;
        close(sock);
        return 1;
    }

    while (true)
    {
        // Принимаем данные с клиента с помощью recv
        // В Unix для тех же целей можно использовать read()
        try
        {
            Data data = Data::ReceiveFrom(client);
            std::cout << data << std::endl;
        }
        catch ( const std::exception& )
        {
            std::cerr << "Could not receive data!" << std::endl;
            break;
        }
    }

    // Закрываем сокет, вместе с ним соединение
    close(sock);

    Cleanup();
    return 0;
}