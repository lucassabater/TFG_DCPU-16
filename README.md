# TFG_DCPU-16
 
Emulator and toolchain for the DCPU-16, the fictional 16-bit CPU designed by Notch (Markus Persson) for the game 0x10c. This project is a Final Year Project (TFG) written in C that includes a cycle-accurate graphical emulator.
 
### Features
 
* Cycle-accurate emulator of the complete DCPU-16 instruction set (spec 1.7): basic operations (`SET`, `ADD`, `SUB`, `MUL`, `MLI`, `DIV`, `DVI`, `MOD`, `MDI`, `AND`, `BOR`, `XOR`, `SHR`, `ASR`, `SHL`, conditional jumps `IFB/IFC/IFE/IFN/IFG/IFA/IFL/IFU`, `ADX`, `SUX`, `STI`, `STD`) and special instructions (`JSR`, `INT`, `IAG`, `IAS`, `RFI`, `IAQ`, `HWN`, `HWQ`, `HWI`).
* Hardware bus with connectable devices (interface `HWI`/`HWQ`/`HWN`).
* LEM1802 display (128×96 px, 32×12 characters) rendered with SDL2, including border color and cursor blinking.
* Generic keyboard and generic clock as hardware peripherals.
* Graphical interface built with SDL2 + Nuklear (immediate-mode GUI): ROM loading via native file dialog (tinyfiledialogs), Play/Pause control, CPU Reset, and emulation speed control (1,000–300,000 Hz).
### Repository Structure
 
```
├── include/        Emulator headers (dcpu16, hardware, LEM1802, keyboard, clock, window...)
├── lib/             Third-party dependencies: SDL2 (Windows/Linux), Nuklear, tinyfiledialogs
├── specifications/  DCPU-16 reference documentation
├── src/             Emulator implementation and peripherals
├── test/            Project tests
├── main.c           Graphical emulator entry point
└── CMakeLists.txt   Build configuration (Windows and Linux)
```
 
### Downloads (Recommended)
 
Pre-compiled `.zip` packages are available in the [Releases](https://github.com/lucassabater/TFG_DCPU-16/releases) section of this repository, ready to use without compilation:
 
* **Windows** — includes `dcpu_emulator.exe`, `dcpu_assembler.exe`, and `SDL2.dll`.
* **Linux** — includes `dcpu_emulator`, `dcpu_assembler`, and required SDL2 shared libraries. Simply extract the zip for your system and run the binary (on Linux, you may need to grant execution permissions with `chmod +x dcpu_emulator dcpu_assembler`).
### Building from Source
 
#### Requirements
 
* CMake ≥ 3.10
* C11-compatible compiler (GCC/MinGW on Windows, GCC/Clang on Linux)
* SDL2 (already precompiled in `lib/SDL2` for Windows and `lib/SDL2_linux` for Linux)
#### Steps
 
```bash
mkdir build && cd build
cmake ..
cmake --build .
```
 
This generates two executables:
 
* `dcpu_emulator` — the emulator with graphical interface. On Windows, `SDL2.dll` is automatically copied alongside the executable. On Linux, libraries from `lib/SDL2_linux` are copied with the binary and linked via `rpath`.
### Usage
 
#### Emulator
 
Run `dcpu_emulator` and use the menu:
 
* **File → Open ROM...** to load a pre-assembled binary (`.bin`/`.txt`).
* **Run → Play/Pause Emulation**, **Reset CPU**, and the **Speed (Hz)** slider to control execution.
### Author
 
Lucas Sabater
