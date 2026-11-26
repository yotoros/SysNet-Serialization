cmake_minimum_required(VERSION 3.12) # С версии 3.12 поддерживается C++20

# Имя проекта и используемые языки
project(sysnet CXX)

# Устанавливаем стандарт C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Указываем, куда сохранять артефакты сборки
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

# Включаемые каталоги, в которых находятся наши заголовочные файлы
include_directories(AFTER
    ${CMAKE_SOURCE_DIR}/include
)

# Указываем дополнительные флаги компилятора:
# - не допускаем никаких предупреждений
# - используем строгое следование стандарту
# - все предупреждения считаем ошибками
if (WIN32)
add_compile_options(/W4 /permissive- /WX)
else()
add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Подпроекты
add_subdirectory(client_server)
add_subdirectory(serialization)
