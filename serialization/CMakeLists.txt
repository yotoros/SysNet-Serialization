# Подпроект, представляющий простое взаимодействие "один клиент - сервер"
project(serialization)

add_executable(server_serialization
    server.cpp
)

add_executable(client_serialization
    client.cpp
)