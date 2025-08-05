cmake_minimum_required(VERSION 3.24)
project(SearchAndCollect2 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(SOURCES
    src/main.c
    src/common.c                include/SearchAndCollect2/common.h
    src/filescanner.c           include/SearchAndCollect2/filescanner.h
    include/SearchAndCollect2/error_codes.h

    src/lklist.c                include/lklist.h
)

add_executable(SearchAndCollect2 ${SOURCES})

target_include_directories(SearchAndCollect2 PRIVATE "include/")

target_link_libraries(SearchAndCollect2 PRIVATE bcrypt)
