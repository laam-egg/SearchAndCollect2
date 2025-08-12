cmake_minimum_required(VERSION 3.24)
project(SearchAndCollect2 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

##########################
##### STATIC LINKING #####
#### so that it could ####
## run on older systems ##
## without VC redist... ##
##########################

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

set(SOURCES
    src/main.c
    src/common.c                include/SearchAndCollect2/common.h
	src/config.c				include/SearchAndCollect2/config.h
    src/filescanner.c           include/SearchAndCollect2/filescanner.h
    src/dfsfilescanner.c        include/SearchAndCollect2/dfsfilescanner.h
    src/pe.c                    include/SearchAndCollect2/pe.h
    src/hashing.c               include/SearchAndCollect2/hashing.h
    src/collect.c               include/SearchAndCollect2/collect.h
    src/error_codes.c           include/SearchAndCollect2/error_codes.h

    src/lklist.c                include/lklist.h
)

set(INCLUDES
    include/
)

set(LIBRARIES
    bcrypt
)



# add_executable(test_lklist ${SOURCES})
# target_compile_definitions(test_lklist PRIVATE LK_RUN_TEST)
# target_include_directories(test_lklist PRIVATE ${INCLUDES})
# target_link_libraries(test_lklist PRIVATE ${LIBRARIES})





# This program makes extensive use of reusable stack buffers
# for reducing heap allocations. As such, increasing the
# stack size is required so that the OS won't issue a
# "stack overflow" error in normal circumstances.
#
# 16 MB is just a generous estimate; the program might actually
# use less at most.

if (MSVC)
    add_link_options(/STACK:16777216)
elseif (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    if (WIN32)
        add_link_options("-Wl,--stack,16777216")
    else()
        # Linux, BSD, macOS: raise stack at runtime via setrlimit()
        # add_compile_definitions(SET_STACK_AT_RUNTIME)
		# But this program only runs on Windows so...
		message(FATAL_ERROR "This program can only be built on Windows.")
    endif()
endif()

add_executable(SearchAndCollect2 ${SOURCES})
target_include_directories(SearchAndCollect2 PRIVATE ${INCLUDES})
target_link_libraries(SearchAndCollect2 PRIVATE ${LIBRARIES})

##########################
##### STATIC LINKING #####
#### so that it could ####
## run on older systems ##
## without VC redist... ##
##########################

if(MSVC)
    target_link_options(SearchAndCollect2 PRIVATE /SUBSYSTEM:CONSOLE,5.01)
endif()
if(MINGW)
    target_link_options(SearchAndCollect2 PRIVATE -static -static-libgcc)
endif()
