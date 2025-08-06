cmake_minimum_required(VERSION 3.24)
project(SearchAndCollect2 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(SOURCES
    src/main.c
    src/common.c                include/SearchAndCollect2/common.h
    src/filescanner.c           include/SearchAndCollect2/filescanner.h
    src/dfsfilescanner.c        include/SearchAndCollect2/dfsfilescanner.h
    include/SearchAndCollect2/error_codes.h

    src/lklist.c                include/lklist.h
)

set(INCLUDES
    include/
)

set(LIBRARIES
    bcrypt
)

add_executable(SearchAndCollect2 ${SOURCES})
target_include_directories(SearchAndCollect2 PRIVATE ${INCLUDES})
target_link_libraries(SearchAndCollect2 PRIVATE ${LIBRARIES})

add_executable(test_lklist ${SOURCES})
target_compile_definitions(test_lklist PRIVATE LK_RUN_TEST)
target_include_directories(test_lklist PRIVATE ${INCLUDES})
target_link_libraries(test_lklist PRIVATE ${LIBRARIES})
