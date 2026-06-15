cmake_minimum_required(VERSION 3.10)
project(dcpu_emulator C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Core source files
set(SOURCES
        main.c
        src/dcpu16.c
        src/hardware_device.c
        src/emulation_utils.c
        src/generic_clock.c
        src/keyboard.c
        src/lem1802.c
        src/lem_data.c
        lib/TINYFILEDIALOGS/tinyfiledialogs.c
        src/window.c
        src/emulator.c
)

# Core header files
set(HEADERS
        include/dcpu16.h
        include/hardware_device.h
        include/emulation_utils.h
        include/generic_clock.h
        include/keyboard.h
        include/lem1802.h
        include/lem_data.h
        include/window.h
        include/emulator.h
)

# Executable targets
add_executable(dcpu_emulator ${SOURCES} ${HEADERS})
add_executable(dcpu_assembler assembler.c)

# Include directories configuration
target_include_directories(dcpu_emulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(dcpu_emulator PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/NUKLEAR
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/TINYFILEDIALOGS
)

# Windows deployment
if(WIN32)
    set(SDL2_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/SDL2")
    target_include_directories(dcpu_emulator PUBLIC ${SDL2_PATH}/include)
    target_link_directories(dcpu_emulator PUBLIC ${SDL2_PATH}/lib)
    target_link_libraries(dcpu_emulator mingw32 SDL2main SDL2)

    # Copy runtime library next to the executable
    file(COPY "${SDL2_PATH}/bin/SDL2.dll" DESTINATION "${CMAKE_BINARY_DIR}")

    # Linux deployment
elseif(UNIX AND NOT APPLE)
    set(SDL2_LINUX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/SDL2_linux")
    target_include_directories(dcpu_emulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib/SDL2/include)
    target_link_directories(dcpu_emulator PUBLIC ${SDL2_LINUX_PATH})
    target_link_libraries(dcpu_emulator SDL2 m)

    # Setup relative runtime path and copy shared objects
    set_target_properties(dcpu_emulator PROPERTIES LINK_FLAGS "-Wl,-rpath,'$ORIGIN/lib/SDL2_linux'")
    file(COPY "${SDL2_LINUX_PATH}" DESTINATION "${CMAKE_BINARY_DIR}/lib")
endif()