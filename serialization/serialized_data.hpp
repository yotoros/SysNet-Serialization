#pragma once

#ifndef SERIALIZED_DATA_HPP
#define SERIALIZED_DATA_HPP

#include <cstdint>
#include <exception>
#include <string>
#include <iostream>
#include <network_common.hpp>

// Доступные цвета текста (это ANSI-коды для консоли)
enum class Color : std::int32_t
{
    DEFAULT = 0,
    RED = 91,
    GREEN = 92,
    YELLOW = 93
};

// Переводит 64-битное число из порядка хоста в Big-Endian (сетевой порядок).
// Готовых htobe64/be64toh в Windows нет, поэтому собираем их из htonl.
inline std::uint64_t ToBigEndian64(std::uint64_t value)
{
    // Если хост уже Big-Endian, число менять не нужно
    if (htonl(1) == 1)
    {
        return value;
    }

    std::uint32_t high = htonl(static_cast<std::uint32_t>(value >> 32));
    std::uint32_t low = htonl(static_cast<std::uint32_t>(value & 0xFFFFFFFFu));
    return (static_cast<std::uint64_t>(low) << 32) | high;
}

// Обратное преобразование делается точно так же
inline std::uint64_t FromBigEndian64(std::uint64_t value)
{
    return ToBigEndian64(value);
}

class Data
{
public:
    Data(std::string line, Color color = Color::DEFAULT)
        : line_(std::move(line)), color_(color)
    {}

    // Порядок отправки: 1) размер строки, 2) цвет, 3) сама строка
    void SendTo(socket_t sock)
    {
        // 1. Размер строки в Big-Endian
        std::uint64_t netsize = ToBigEndian64(line_.size());
        ssize_t bytes = send(sock, reinterpret_cast<const char*>(&netsize),
                             static_cast<netsize_t>(sizeof(netsize)), 0);
        if (bytes <= 0)
        {
            throw std::exception{};
        }

        // 2. Цвет в Big-Endian
        std::uint32_t netcolor = htonl(static_cast<std::uint32_t>(color_));
        bytes = send(sock, reinterpret_cast<const char*>(&netcolor),
                     static_cast<netsize_t>(sizeof(netcolor)), 0);
        if (bytes <= 0)
        {
            throw std::exception{};
        }

        // 3. Сама строка (если она не пустая)
        if (!line_.empty())
        {
            bytes = send(sock, line_.c_str(),
                         static_cast<netsize_t>(line_.size()), 0);
            if (bytes <= 0)
            {
                throw std::exception{};
            }
        }
    }

    static Data ReceiveFrom(socket_t sock)
    {
        // 1. Размер строки
        std::uint64_t netsize = 0;
        ssize_t bytes = recv(sock, reinterpret_cast<char*>(&netsize),
                             static_cast<netsize_t>(sizeof(netsize)), 0);
        if (bytes <= 0)
        {
            throw std::exception{};
        }
        std::size_t size = static_cast<std::size_t>(FromBigEndian64(netsize));

        // 2. Цвет
        std::uint32_t netcolor = 0;
        bytes = recv(sock, reinterpret_cast<char*>(&netcolor),
                     static_cast<netsize_t>(sizeof(netcolor)), 0);
        if (bytes <= 0)
        {
            throw std::exception{};
        }
        Color color = static_cast<Color>(static_cast<std::int32_t>(ntohl(netcolor)));

        // 3. Сама строка (если она не пустая)
        std::string line(size, '\0');
        if (size > 0)
        {
            bytes = recv(sock, line.data(), static_cast<netsize_t>(size), 0);
            if (bytes <= 0)
            {
                throw std::exception{};
            }
        }

        return Data(line, color);
    }

    const std::string& GetLine() const
    {
        return line_;
    }

    Color GetColor() const
    {
        return color_;
    }

    friend std::ostream& operator<<( std::ostream& s, const Data& data )
    {
        return s << data.line_;
    }

private:
    std::string line_;
    Color color_;
};

#endif // SERIALIZED_DATA_HPP