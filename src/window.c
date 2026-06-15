/**
 * @file window.c
 * @brief Window management, event polling, and Immediate Mode GUI (Nuklear) integration.
 */

#include "window.h"
#include "tinyfiledialogs.h"

// Nuklear configuration and implementation macros
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_RENDERER_IMPLEMENTATION

#include "nuklear.h"
#include "nuklear_sdl_renderer.h"

#define SCALE_FACTOR 6
#define BORDER_SIZE  8

/**
 * @brief Initializes the SDL2 window, hardware renderer, and GUI context.
 * * @param win Pointer to the MainWindow instance.
 * @return true on success, false on failure.
 */
bool window_init(MainWindow *win) {
    win->window_w = (LEM1802_WIDTH + BORDER_SIZE * 2) * SCALE_FACTOR;
    win->window_h = (LEM1802_HEIGHT + BORDER_SIZE * 2) * SCALE_FACTOR;

    win->window = SDL_CreateWindow("DCPU-16 Studio",
                                   SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   win->window_w, win->window_h,
                                   SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!win->window) return false;

    win->renderer = SDL_CreateRenderer(win->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // Allocate streaming texture for high-frequency pixel updates
    win->lem_texture = SDL_CreateTexture(win->renderer,
                                         SDL_PIXELFORMAT_RGBA8888,
                                         SDL_TEXTUREACCESS_STREAMING,
                                         LEM1802_WIDTH, LEM1802_HEIGHT);

    win->ctx = nk_sdl_init(win->window, win->renderer);

    struct nk_font_atlas *atlas;
    nk_sdl_font_stash_begin(&atlas);
    nk_sdl_font_stash_end();

    return true;
}

/**
 * @brief Safely destroys all graphical resources and shuts down the GUI.
 * * @param win Pointer to the MainWindow instance.
 */
void window_cleanup(MainWindow *win) {
    nk_sdl_shutdown();
    SDL_DestroyTexture(win->lem_texture);
    SDL_DestroyRenderer(win->renderer);
    SDL_DestroyWindow(win->window);
}

/**
 * @brief Constructs the Immediate Mode GUI layout for the current frame.
 */
static void draw_nuklear_ui(MainWindow *win, EmulatorState *emu) {
    if (nk_begin(win->ctx, "IDE", nk_rect(0, 0, win->window_w, win->window_h), NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {

        nk_menubar_begin(win->ctx);
        nk_layout_row_begin(win->ctx, NK_STATIC, 25, 3);

        // File Menu
        nk_layout_row_push(win->ctx, 50);
        if (nk_menu_begin_label(win->ctx, "File", NK_TEXT_CENTERED, nk_vec2(200, 200))) {
            nk_layout_row_dynamic(win->ctx, 25, 1);

            if (nk_menu_item_label(win->ctx, "Open ROM...", NK_TEXT_LEFT)) {
                const char *filters[2] = { "*.bin", "*.txt" };
                const char *filepath = tinyfd_openFileDialog("Select a ROM", "", 2, filters, "ROM Files", 0);

                if (filepath != NULL) {
                    emu_load_rom(emu, filepath);
                }
            }

            if (nk_menu_item_label(win->ctx, "Exit", NK_TEXT_LEFT)) {
                // Safely inject a quit event into the SDL queue
                SDL_Event quit_event;
                quit_event.type = SDL_QUIT;
                SDL_PushEvent(&quit_event);
            }
            nk_menu_end(win->ctx);
        }

        // Run Menu
        nk_layout_row_push(win->ctx, 50);
        if (nk_menu_begin_label(win->ctx, "Run", NK_TEXT_CENTERED, nk_vec2(200, 200))) {
            nk_layout_row_dynamic(win->ctx, 25, 1);

            const char *play_text = emu->is_playing ? "Pause Emulation" : "Play Emulation";
            if (nk_menu_item_label(win->ctx, play_text, NK_TEXT_LEFT)) {
                if (emu->rom_loaded) {
                    emu->is_playing = !emu->is_playing;

                    if (emu->is_playing) {
                        keyboard_clear_buffer(&emu->keyboard);
                    }
                }
            }

            if (nk_menu_item_label(win->ctx, "Reset CPU", NK_TEXT_LEFT)) {
                emu_reset(emu);
            }

            nk_menu_item_label(win->ctx, "Speed (Hz):", NK_TEXT_LEFT);

            // Bridge uint32_t speed to Nuklear's expected int pointer safely
            int temp_speed = (int)emu->speed_hz;
            nk_slider_int(win->ctx, 1000, &temp_speed, 300000, 1000);
            emu->speed_hz = (uint32_t)temp_speed;

            nk_menu_end(win->ctx);
        }

        nk_layout_row_end(win->ctx);
        nk_menubar_end(win->ctx);

        // Center Screen Rendering Area
        nk_layout_row_dynamic(win->ctx, 20, 1);
        nk_spacing(win->ctx, 1);

        float screen_ratios[] = {0.1f, 0.8f, 0.1f};
        nk_layout_row(win->ctx, NK_DYNAMIC, LEM1802_HEIGHT * SCALE_FACTOR, 3, screen_ratios);
        nk_spacing(win->ctx, 1);
        nk_image(win->ctx, nk_image_ptr(win->lem_texture));
        nk_spacing(win->ctx, 1);
    }
    nk_end(win->ctx);
}

/**
 * @brief Polls system events and dispatches them to GUI and Emulator handlers.
 * * @param win Pointer to the MainWindow instance.
 * @param emu Pointer to the EmulatorState instance.
 * @return false if a quit request was intercepted, true otherwise.
 */
bool window_process_events(MainWindow *win, EmulatorState *emu) {
    SDL_Event event;
    nk_input_begin(win->ctx);

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) return false;

        nk_sdl_handle_event(&event);

        if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
            emu_handle_keydown(emu, event.key.keysym.sym);
        } else if (event.type == SDL_KEYUP) {
            emu_handle_keyup(emu, event.key.keysym.sym);
        }
    }
    nk_input_end(win->ctx);

    return true;
}

/**
 * @brief Composes and presents the final graphical frame.
 * * @param win Pointer to the MainWindow instance.
 * @param emu Pointer to the EmulatorState instance.
 */
void window_render(MainWindow *win, EmulatorState *emu) {
    draw_nuklear_ui(win, emu);

    lem1802_update(&emu->screen, &emu->cpu, win->lem_texture);

    uint32_t border_rgba = lem1802_get_border_color(&emu->screen, &emu->cpu);

    uint8_t r = (border_rgba >> 24) & 0xFF;
    uint8_t g = (border_rgba >> 16) & 0xFF;
    uint8_t b = (border_rgba >> 8) & 0xFF;

    SDL_SetRenderDrawColor(win->renderer, r, g, b, 255);
    SDL_RenderClear(win->renderer);

    nk_sdl_render(NK_ANTI_ALIASING_ON);

    SDL_RenderPresent(win->renderer);
}

/**
 * @brief Clears the internal key buffer and releases all currently pressed keys.
 * @param kb Pointer to the GenericKeyboard instance.
 */
void keyboard_clear_buffer(GenericKeyboard *kb) {
    kb->head = 0;
    kb->tail = 0;
    kb->count = 0;
    memset(kb->is_pressed, 0, sizeof(kb->is_pressed));
}