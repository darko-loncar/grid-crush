#include "tetroid.h"
#include "draw.h"
#include "sound.h"
#include "ui.h"
#include "preview.h"
#include "input.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define WINDOW_W 10
#define WINDOW_H 20
#define HIDDEN_ROWS 2
#define BOARD_ROWS (WINDOW_H + HIDDEN_ROWS)
#define WINDOW_PIXEL_W (WINDOW_W * BLOCK_SIZE + 300)
#define WINDOW_PIXEL_H (WINDOW_H * BLOCK_SIZE + 30)
#define FRAME_TIME_MS 16

static SDL_Window *window;
static SDL_Event event;
static SDL_Texture *background_grain;
static Matrix playfield;
static Tetromino tActive;
static PreviewQueue preview_queue;
static GameState game_state = GAME_MENU;
static bool running = true;
static InputContext input_context;
static int gravity;
static int lines;
static int score;
static int level = 1;
static int lines_to_level = 10;
static Uint64 last_tick;
static Uint64 gravity_accumulator;
static bool initSDL(void);
static bool startNewGame(void);
static void lockActivePiece(void);
static void renderFrame(void);
static void clearGameMatrices(void);
static void cleanup(void);
static void shutdown(void);

int main(void) {
#if defined(__linux__)
    setenv("__NV_DISABLE_EXPLICIT_SYNC", "1", 1); // Nvidia Fix on Linux
#endif

    if (!initSDL()) {
        shutdown();
        return 1;
    }
    if (!sound_init()) {
        SDL_Log("Continuing without sound.");
    }
    if (!initDraw(window)) {
        shutdown();
        return 1;
    }

    background_grain = createLightGrainTexture(WINDOW_PIXEL_W, WINDOW_PIXEL_H);
    srand((unsigned int)time(NULL));
    last_tick = SDL_GetTicks();
    input_context = (InputContext){
        .running = &running,
        .game_state = &game_state,
        .active_piece = &tActive,
        .playfield = &playfield,
        .gravity = &gravity,
        .level = &level,
        .last_tick = &last_tick,
        .start_new_game = startNewGame,
        .lock_active_piece = lockActivePiece
    };

    while (running) {
        Uint64 frame_start = SDL_GetTicks();
        while (SDL_PollEvent(&event)) {
            input_handle_event(&event, &input_context);
        }

        Uint64 now = SDL_GetTicks();
        if (game_state == GAME_PLAYING) {
            gravity_accumulator += now - last_tick;
            if (gravity_accumulator >= (Uint64)gravity) {
                if (can_move_down(&tActive, &playfield)) {
                    tActive.y_pos++;
                } else {
                    lockActivePiece();
                }
                gravity_accumulator = 0;
            }
        }
        last_tick = now;

        renderFrame();
        SDL_RenderPresent(renderer);

        Uint64 frame_elapsed = SDL_GetTicks() - frame_start;
        if (frame_elapsed < FRAME_TIME_MS) {
            SDL_Delay((Uint32)(FRAME_TIME_MS - frame_elapsed));
        }
    }

    shutdown();
    return 0;
}

static bool initSDL(void) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow("Gridcrush", WINDOW_PIXEL_W, WINDOW_PIXEL_H, 0);
    if (!window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        return false;
    }
    return true;
}

static bool startNewGame(void) {
    clearGameMatrices();

    playfield = matrix_create_empty(BOARD_ROWS, WINDOW_W);
    if (!playfield.cells) {
        SDL_Log("Could not allocate the playfield.");
        game_state = GAME_MENU;
        return false;
    }

    tActive = generate_random_piece();
    if (!tActive.matrix.cells || !preview_queue_init(&preview_queue)) {
        SDL_Log("Could not allocate a game piece.");
        clearGameMatrices();
        game_state = GAME_MENU;
        return false;
    }

    lines = 0;
    score = 0;
    level = 1;
    lines_to_level = 10;
    gravity = get_level_gravity(level);
    gravity_accumulator = 0;
    last_tick = SDL_GetTicks();
    game_state = GAME_PLAYING;
    return true;
}

static void lockActivePiece(void) {
    sound_play_piece_lock();
    bool locked_in_hidden_rows = false;
    for (int row = 0; row < tActive.matrix.rows; row++) {
        for (int col = 0; col < tActive.matrix.cols; col++) {
            const MatrixCell *cell = &tActive.matrix.cells[row * tActive.matrix.cols + col];
            if (cell->active && tActive.y_pos + row < HIDDEN_ROWS) {
                locked_in_hidden_rows = true;
            }
        }
    }

    matrix_merge(&tActive.matrix, &playfield, tActive.y_pos, tActive.x_pos);
    int lines_removed = remove_completed_rows(&playfield);
    score += calculate_score(lines_removed, level);

    if (locked_in_hidden_rows) {
        game_state = GAME_OVER;
        return;
    }

    update_level(lines_removed, &lines, &lines_to_level, &level, &gravity);
    matrix_free(&tActive.matrix);
    tActive = (Tetromino){0};
    bool got_next_piece = preview_queue_advance(&preview_queue, &tActive);
    gravity_accumulator = 0;

    if (!got_next_piece || !tActive.matrix.cells ||
        matrix_overlap(&tActive.matrix, &playfield, tActive.y_pos, tActive.x_pos, NULL, NULL)) {
        game_state = GAME_OVER;
    }
}

static void renderFrame(void) {
    drawBackgroundGradient(WINDOW_PIXEL_W, WINDOW_PIXEL_H);
    if (background_grain) {
        SDL_FRect grain_bounds = {0.0f, 0.0f, (float)WINDOW_PIXEL_W, (float)WINDOW_PIXEL_H};
        SDL_RenderTexture(renderer, background_grain, NULL, &grain_bounds);
    }


    Matrix empty_playfield = {WINDOW_H, WINDOW_W, NULL};
    drawPlayfield(&empty_playfield, 15, 15);

    if (game_state == GAME_PLAYING || game_state == GAME_PAUSED ||
        game_state == GAME_OVER) {
        SDL_Rect playfield_clip = {
            15, 15, WINDOW_W * BLOCK_SIZE, WINDOW_H * BLOCK_SIZE
        };
        SDL_SetRenderClipRect(renderer, &playfield_clip);
        int board_y_offset = 15 - HIDDEN_ROWS * BLOCK_SIZE;
        drawMatrix(&playfield, 15, board_y_offset);
        if (tActive.matrix.cells) {
            int ghost_y = tActive.y_pos;
            while (!matrix_overlap(&tActive.matrix, &playfield, ghost_y + 1, tActive.x_pos, NULL, NULL)) {
                ghost_y++;
            }
            if (tetromino_has_visible_cell(&tActive, HIDDEN_ROWS)) {
                drawGhostPiece(&tActive.matrix, tActive.x_pos * BLOCK_SIZE + 15, (ghost_y - HIDDEN_ROWS) * BLOCK_SIZE + 15);
            }
            drawMatrix(&tActive.matrix, tActive.x_pos * BLOCK_SIZE + 15, (tActive.y_pos - HIDDEN_ROWS) * BLOCK_SIZE + 15);
        }
        SDL_SetRenderClipRect(renderer, NULL);

        preview_queue_draw(&preview_queue);
    }

    UiView ui_view = {
        .menu = game_state == GAME_MENU,
        .paused = game_state == GAME_PAUSED,
        .game_over = game_state == GAME_OVER,
        .new_game_hovered = input_pointer_in_rect(UI_PANEL_X, UI_NEW_GAME_Y, UI_BUTTON_W, UI_BUTTON_H),
        .quit_hovered = input_pointer_in_rect(UI_PANEL_X, UI_QUIT_Y, UI_BUTTON_W, UI_BUTTON_H),
        .score = score,
        .lines = lines,
        .level = level
    };
    ui_draw(&ui_view);
}

static void clearGameMatrices(void) {
    matrix_free(&playfield);
    matrix_free(&tActive.matrix);
    preview_queue_clear(&preview_queue);
}

static void cleanup(void) {
    if (background_grain) {
        SDL_DestroyTexture(background_grain);
        background_grain = NULL;
    }
    clearGameMatrices();
}

static void shutdown(void) {
    cleanup();
    destroyDraw();
    sound_shutdown();
    if (window) {
        SDL_DestroyWindow(window);
        window = NULL;
    }
    SDL_Quit();
}
