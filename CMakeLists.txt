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
)

set(HEADERS
        include/dcpu16.h
        include/hardware_device.h
        include/emulation_utils.h
        include/generic_clock.h
        include/keyboard.h
)

add_executable(dcpu_emulator ${SOURCES} ${HEADERS})

target_include_directories(dcpu_emulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SDL2_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/SDL2")

target_include_directories(dcpu_emulator PUBLIC ${SDL2_PATH}/include)

target_link_directories(dcpu_emulator PUBLIC ${SDL2_PATH}/lib)

target_link_libraries(dcpu_emulator mingw32 SDL2main SDL2)

file(COPY "${SDL2_PATH}/bin/SDL2.dll" DESTINATION "${CMAKE_BINARY_DIR}")