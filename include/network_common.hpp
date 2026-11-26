#pragma once

// Данный заголовочный файл нам будет нужен практически всегда, в нем
// определены необходимые общие вызовы для реализации IPC посредством
// сокетов. Определения из данного заголовочного файла должны быть
// платформо-независимыми, чтобы и на Windows, и на Linux мы могли
// программировать одинаково

#ifndef NETWORK_COMMON_H
#define NETWORK_COMMON_H


#if defined(_WIN32) || defined(WIN32)
// Это относится к Windows

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

// Определяем нужные типы
#ifdef _MSC_VER
using ssize_t = int;     // В Windows этого типа нет
#endif
using socket_t = SOCKET; // В Windows тип сокета это обычный дескриптор, но размером с указатель
using netsize_t = int;   // В Windows функции send и receive принимают тип int 

// Обертка для вызова closesocket, предназначенного для закрытия сокета
// Добавлен ради кроссплатформенности
void close(socket_t socket)
{
    closesocket(socket);
}

// Этот вызов нужен, чтобы подгрузился WinAPI
#define Startup() \
WSADATA wsaData{}; \
do if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) \
{ \
    std::cout << "WSAStartup failed" << std::endl; \
    return 1; \
} while (0)

// Этот вызов нужен, чтобы WinAPI выгрузился
#define Cleanup() WSACleanup()

#else // UNIX
// Это относится к Unix

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define INVALID_SOCKET -1 // В Unix этого определения для ошибки создания сокета нет, задаем

// Определяем нужные типы
using socket_t = int;     // Тип для сокета в Unix это обобщенный файловый дескриптор
using netsize_t = size_t; // Функции send и receive в Unix принимают тип size_t

// В Unix подгружать ничего не нужно, поэтому этот вызов пустой
inline void Startup() {}

// В Unix выгружать ничего не нужно, поэтому этот вызов пустой
inline void Cleanup() {}

#endif // _WIN32

#endif // NETWORK_COMMON_H
