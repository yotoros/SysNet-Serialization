#pragma once

#ifndef SERIALIZED_DATA_HPP
#define SERIALIZED_DATA_HPP

#include <string>
#include <iostream>
#include <network_common.hpp>

class Data
{
public:
    Data(std::string line)
        : line_(std::move(line))
    {}

    void SendTo(socket_t sock)
    {
        size_t size = line_.size();
        static_assert(sizeof(size) == 8);

        size_t netsize = htobe64(size);
        ssize_t bytes = send(sock, &netsize, sizeof( netsize ), 0 );
        if (bytes <= 0)
        {
            throw std::exception{};
        }

        bytes = send(sock, line_.c_str(), size, 0 );
        if (bytes <= 0)
        {
            throw std::exception{};
        }
    }

    static Data ReceiveFrom(socket_t sock)
    {
        size_t netsize;
        ssize_t bytes = recv(sock, &netsize, sizeof( netsize ), 0 );
        if (bytes <= 0)
        {
            throw std::exception{};
        }

        size_t size = be64toh(netsize);

        std::string line(size, '\0');
        bytes = recv(sock, line.data(), size, 0 );
        if (bytes <= 0)
        {
            throw std::exception{};
        }

        return Data(line);
    }

    friend std::ostream& operator<<( std::ostream& s, const Data& data )
    {
        return s << data.line_;
    }

private:
    std::string line_;
};

#endif // SERIALIZED_DATA_HPP