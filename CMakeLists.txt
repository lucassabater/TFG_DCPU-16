cmake_minimum_required(VERSION 4.1)
project(untitled C)

set(CMAKE_C_STANDARD 11)

add_executable(untitled main.c
        include/dcpu16.h
        src/dcpu16.c
        include/hardware_device.h
        src/hardware_device.c
        include/emulation_utils.h
        src/emulation_utils.c)
