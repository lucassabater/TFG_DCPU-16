# TFG_DCPU-16
 
Emulador y toolchain para la **DCPU-16**, la CPU ficticia de 16 bits diseñada por Notch (Markus Persson) para el juego *0x10c*. Este proyecto es un Trabajo de Fin de Grado (TFG) escrito en C que incluye un emulador gráfico ciclo-exacto.
 
## Características
 
- **Emulador ciclo-exacto** del set de instrucciones completo de la DCPU-16 (spec 1.7): operaciones básicas (`SET`, `ADD`, `SUB`, `MUL`, `MLI`, `DIV`, `DVI`, `MOD`, `MDI`, `AND`, `BOR`, `XOR`, `SHR`, `ASR`, `SHL`, saltos condicionales `IFB/IFC/IFE/IFN/IFG/IFA/IFL/IFU`, `ADX`, `SUX`, `STI`, `STD`) y especiales (`JSR`, `INT`, `IAG`, `IAS`, `RFI`, `IAQ`, `HWN`, `HWQ`, `HWI`).
- **Bus de hardware** con dispositivos conectables (interfaz `HWI`/`HWQ`/`HWN`).
- **Pantalla LEM1802** (128×96 px, 32×12 caracteres) renderizada con SDL2, incluyendo color de borde y parpadeo de cursor.
- **Teclado genérico** y **reloj genérico** (Generic Clock) como periféricos de hardware.
- **Interfaz gráfica** construida con SDL2 + Nuklear (immediate-mode GUI): carga de ROMs mediante diálogo nativo de archivos (tinyfiledialogs), control de Play/Pause, Reset de CPU y control de velocidad de emulación (1.000–300.000 Hz).
## Estructura del repositorio
 
```
├── include/        Cabeceras del emulador (dcpu16, hardware, LEM1802, teclado, reloj, ventana...)
├── lib/             Dependencias de terceros: SDL2 (Windows/Linux), Nuklear, tinyfiledialogs
├── specifications/  Documentación de referencia de la DCPU-16
├── src/             Implementación del emulador y sus periféricos
├── test/            Pruebas del proyecto
├── main.c           Punto de entrada del emulador gráfico
└── CMakeLists.txt   Configuración de compilación (Windows y Linux)
```
 
## Descargas (recomendado)
 
En la sección [Releases](../../releases) de este repositorio se publican paquetes `.zip` listos para usar, sin necesidad de compilar:
 
- **Windows** — incluye `dcpu_emulator.exe`, `dcpu_assembler.exe` y `SDL2.dll`.
- **Linux** — incluye `dcpu_emulator`, `dcpu_assembler` y las librerías compartidas de SDL2 necesarias.
Basta con descomprimir el zip correspondiente a tu sistema y ejecutar el binario (en Linux puede ser necesario dar permisos de ejecución con `chmod +x dcpu_emulator dcpu_assembler`).
 
## Compilación desde el código fuente
 
### Requisitos
 
- CMake ≥ 3.10
- Compilador compatible con C11 (GCC/MinGW en Windows, GCC/Clang en Linux)
- SDL2 (ya se incluye precompilado en `lib/SDL2` para Windows y `lib/SDL2_linux` para Linux)
### Pasos
 
```bash
mkdir build && cd build
cmake ..
cmake --build .
```
 
Esto genera dos ejecutables:
 
- `dcpu_emulator` — el emulador con interfaz gráfica.
En Windows, `SDL2.dll` se copia automáticamente junto al ejecutable. En Linux, las librerías de `lib/SDL2_linux` se copian junto al binario y se enlazan mediante `rpath`.
 
## Uso
 
### Emulador
 
Ejecuta `dcpu_emulator` y usa el menú:
 
- **File → Open ROM...** para cargar un binario (`.bin`/`.txt`) previamente ensamblado.
- **Run → Play/Pause Emulation**, **Reset CPU** y el slider de **Speed (Hz)** para controlar la ejecución.
 
## Autor
 
Lucas Sabater — Trabajo de Fin de Grado.
