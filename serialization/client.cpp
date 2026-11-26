// Это реализация клиента. Для него необходимо:
// - создать файловый дескриптор сокета
// - подключиться по адресу и прикрепить подключение к сокету
// - читать/писать данные по сокету

#include <network_common.hpp>
#include "serialized_data.hpp"

#include <iostream>
#include <string>
#include <cstring>
#include <exception>

int main()
{
    Startup();

    // Создаем файловый дескриптор сокета:
    // - AF_INET означает "Address Family - internet IPv4"
    // - SOCK_STREAM - это потоковый сокет, для взаимодействия по сети будет использоваться
    //                 протокол транспортного уровня TCP (выполняется контроль передачи)
    // - 0 - протокол выбирается автоматически, однако можно указать что-либо конкретное
    socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        std::cerr << "Error creating socket: " << std::strerror( errno ) << std::endl;
        return 1;
    }

    // На этом этапе пользователь вводит адрес сервера, к которому нужно подключиться
    // Например, 127.0.0.1:25565
    std::cout << "Enter IP-address to connect to [xxx.xxx.xxx.xxx:ppppp]:" << std::endl;

    std::string line;
    std::getline(std::cin, line);

    std::string address;
    short port;
    try
    {
        size_t colon = line.find(':');
        if (colon == std::string::npos)
        {
            throw std::exception{};
        }
        address = line.substr(0, colon);
        port = static_cast<short>(std::stoi(line.substr(colon + 1)));
    }
    catch ( const std::exception& )
    {
        // Делаем защиту от дурачка, который неправильно ввел данные
        std::cerr << "Error occured: wrong IP address format" << std::endl;
        close(sock); // При любой ошибке и дальше нужно сначала закрыть сокет, а потом выйти
        return 1;
    }

    // Этими действиями мы преобразуем введенный адрес к нужному внутреннему представлению
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    // Обратите внимание на функцию htons - она расшифровывается как
    // Host TO Network Short - перевести порядок следования байт в числе типа short из хостового в сетевой
    // Сетевой порядок следования байт - это Big Endian, а на хосте с архитектурой x86 - Little Endian.
    addr.sin_port = htons(port);
    inet_pton(AF_INET, address.c_str(), &addr.sin_addr);

    // Подключаемся к серверу по заданному адресу, тем самым закрепляя сокет за сервером
    // Обратите внимание, что мы нашу структуру должны привести к более общему типу sockaddr
    int rv = connect(sock, reinterpret_cast< sockaddr* >( &addr ), sizeof(sockaddr_in));
    if (rv == -1)
    {
        std::cerr << "Error connecting to server: " << std::strerror( errno ) << std::endl;
        close(sock);
        return 1;
    }

    std::cout << "Connected, start writing messages!" << std::endl;

    while (true)
    {
        // Вводим значения, пока не отправим Ctrl + D (Ctrl + C) или не введем .exit
        std::getline(std::cin, line);
        if (line == ".exit" || std::cin.eof())
        {
            break;
        }

        // Отправляем данные на сервер с помощью send
        // В Unix для тех же целей можно использовать write()
        Data data(line);
        try
        {
            data.SendTo(sock);
        }
        catch(const std::exception&)
        {
            std::cerr << "Could not send data!" << std::endl;
            break;
        }
    }

    // Закрываем сокет, вместе с ним соединение
    close(sock);

    Cleanup();
    return 0;
}