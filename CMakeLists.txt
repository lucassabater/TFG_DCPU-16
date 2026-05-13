cmake_minimum_required(VERSION 3.10)
project(dcpu_emulator C)

set(CMAKE_C_STANDARD 11)

set(SOURCES
        main.c
        src/dcpu16.c
        src/hardware_device.c
        src/emulation_utils.c
        src/generic_clock.c
        src/keyboard.c
        src/keyboard.h
)

set(HEADERS
        include/dcpu16.h
        include/hardware_device.h
        include/emulation_utils.h
        include/generic_clock.h
        src/keyboard.c
        src/keyboard.h
)
add_executable(dcpu_emulator ${SOURCES} ${HEADERS})

target_include_directories(dcpu_emulator PUBLIC include)