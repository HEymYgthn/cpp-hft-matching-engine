cmake_minimum_required(VERSION 3.20)
project(CppHftMatchingEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/O2 /arch:AVX2)
else()
    add_compile_options(-O3 -march=native -flto -Wall -Wextra -pedantic)
endif()

include_directories(include)

add_executable(hft_engine
    src/main.cpp
    src/order_book.cpp
)
